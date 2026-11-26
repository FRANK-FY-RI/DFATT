import math
import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch
from matplotlib.path import Path

N = int(input())
M = int(input())

Symbols = [int(value) for value in input().split()]

with open("data/symbol_names.tsv", encoding="ascii") as symbol_file:
    SYMBOL_NAMES = [None] * 256
    for line in symbol_file:
        code, name = line.rstrip("\n").split("\t")
        SYMBOL_NAMES[int(code)] = name

edges = []
states = []  # states in the order they appear (first one is the default start)

for i in range(N):
    temp = input().split()
    u = temp[0]
    states.append(u)

    for j in range(M):
        edges.append((u, temp[j+1], SYMBOL_NAMES[Symbols[j]]))


def read_optional_line():
    try:
        return input().split()
    except EOFError:
        return []

start_line = read_optional_line()
final_line = read_optional_line()

start_state = start_line[0] if start_line else states[0]
final_states = set(final_line)


G = nx.DiGraph()
G.add_nodes_from(states)
G.add_node(start_state)
G.add_nodes_from(final_states)

for u, v, symbol in edges:
    if G.has_edge(u, v):
        G[u][v]["label"] += "," + symbol
    else:
        G.add_edge(u, v, label=symbol)

def compute_layout(graph):
    try:
        from networkx.drawing.nx_agraph import graphviz_layout
        raw = graphviz_layout(
            graph, prog="dot",
            args="-Grankdir=LR -Gnodesep=0.8 -Granksep=1.2",
        )
    except Exception:
        try:
            raw = nx.kamada_kawai_layout(graph)
        except Exception:
            raw = nx.circular_layout(graph)

    nodes = list(raw)
    pts = {n: (float(raw[n][0]), float(raw[n][1])) for n in nodes}

    min_dist = None
    for i in range(len(nodes)):
        for j in range(i + 1, len(nodes)):
            d = math.dist(pts[nodes[i]], pts[nodes[j]])
            if d > 1e-9 and (min_dist is None or d < min_dist):
                min_dist = d

    target = 1.8  # distance (in drawing units) between the closest two states
    k = target / min_dist if min_dist else 1.0
    return {n: (p[0] * k, p[1] * k) for n, p in pts.items()}


pos = compute_layout(G)

R = 0.32          # state radius
RING = R * 1.18   # outer ring radius for final states
START_COLOR = "#b7e4c7"
STATE_COLOR = "#cfe8fc"

xs = [p[0] for p in pos.values()]
ys = [p[1] for p in pos.values()]
cx, cy = sum(xs) / len(xs), sum(ys) / len(ys)

margin = 1.8
x_min, x_max = min(xs) - margin, max(xs) + margin
y_min, y_max = min(ys) - margin, max(ys) + margin

inch_per_unit = min(1.0, 30.0 / max(x_max - x_min, y_max - y_min))
fig_w = max(6.0, (x_max - x_min) * inch_per_unit)
fig_h = max(5.0, (y_max - y_min) * inch_per_unit)

fig, ax = plt.subplots(figsize=(fig_w, fig_h))
ax.set_xlim(x_min, x_max)
ax.set_ylim(y_min, y_max)
ax.set_aspect("equal")
ax.set_axis_off()

nodelist = list(G.nodes())
cmap = plt.get_cmap("tab10")
node_color = {n: cmap(i % 10) for i, n in enumerate(nodelist)}


def outward_angle(node):
    x, y = pos[node]
    if math.hypot(x - cx, y - cy) < 1e-9:
        return math.pi / 2
    return math.atan2(y - cy, x - cx)


def radius_of(node):
    return RING if node in final_states else R


def angle_diff(a, b):
    d = (a - b) % (2 * math.pi)
    return min(d, 2 * math.pi - d)


def free_angle(node, extra_blocked=()):
    """Direction around `node` that is farthest from its edges (so start
    arrows and self loops don't run over other transitions)."""
    x, y = pos[node]
    blocked = list(extra_blocked)
    for other in set(G.successors(node)) | set(G.predecessors(node)):
        if other != node:
            ox, oy = pos[other]
            blocked.append(math.atan2(oy - y, ox - x))
    if not blocked:
        return outward_angle(node)
    out = outward_angle(node)
    best, best_score = out, -1.0
    for k in range(24):
        a = 2 * math.pi * k / 24
        score = min(angle_diff(a, b) for b in blocked) - 0.02 * angle_diff(a, out)
        if score > best_score:
            best, best_score = a, score
    return best


start_angle = free_angle(start_state)
loop_angle = {}
for n in nodelist:
    if G.has_edge(n, n):
        loop_angle[n] = free_angle(n, [start_angle] if n == start_state else [])


circles = {}  
for n in nodelist:
    x, y = pos[n]
    outer = None
    if n in final_states:  
        outer = Circle((x, y), RING, fc="white", ec="black", lw=1.5, zorder=2)
        ax.add_patch(outer)
    inner = Circle(
        (x, y), R,
        fc=START_COLOR if n == start_state else STATE_COLOR,
        ec="black", lw=1.5, zorder=3,
    )
    ax.add_patch(inner)
    circles[n] = outer if outer is not None else inner
    ax.text(x, y, str(n), ha="center", va="center",
            fontsize=11, fontweight="bold", zorder=5)


def draw_label(x, y, text, color):
    ax.text(
        x, y, text, ha="center", va="center", fontsize=9, color=color,
        zorder=4,
        bbox=dict(boxstyle="round,pad=0.15", fc="white", ec="none", alpha=0.9),
    )


for u, v, data in G.edges(data=True):
    col = node_color[u]
    label = data["label"]

    if u == v:
        theta = loop_angle[u]
        x, y = pos[u]
        r = radius_of(u)
        spread = math.radians(28)
        p0 = (x + r * math.cos(theta + spread * 0.9), y + r * math.sin(theta + spread * 0.9))
        p3 = (x + r * math.cos(theta - spread * 0.9), y + r * math.sin(theta - spread * 0.9))
        p1 = (x + r * 2.6 * math.cos(theta + spread * 1.6), y + r * 2.6 * math.sin(theta + spread * 1.6))
        p2 = (x + r * 2.6 * math.cos(theta - spread * 1.6), y + r * 2.6 * math.sin(theta - spread * 1.6))
        path = Path([p0, p1, p2, p3], [Path.MOVETO, Path.CURVE4, Path.CURVE4, Path.CURVE4])
        ax.add_patch(FancyArrowPatch(
            path=path, arrowstyle="-|>", mutation_scale=16,
            lw=1.6, color=col, zorder=1,
        ))
        draw_label(x + (r * 1.3 + 0.2) * math.cos(theta),
                   y + (r * 1.3 + 0.2) * math.sin(theta), label, col)
        continue

    # Two states connected both ways get opposite curves so the arrows
    # (and their labels) don't sit on top of each other.
    rad = 0.3 if G.has_edge(v, u) else 0.0
    (x1, y1), (x2, y2) = pos[u], pos[v]
    ax.add_patch(FancyArrowPatch(
        posA=(x1, y1), posB=(x2, y2),
        patchA=circles[u], patchB=circles[v],
        arrowstyle="-|>", mutation_scale=16, lw=1.6, color=col,
        connectionstyle=f"arc3,rad={rad}", shrinkA=0, shrinkB=0, zorder=1,
    ))
    dx, dy = x2 - x1, y2 - y1
    lx = (x1 + x2) / 2 + 0.5 * rad * dy
    ly = (y1 + y2) / 2 - 0.5 * rad * dx
    draw_label(lx, ly, label, col)


# ---- start arrow ----------------------------------------------------------
sx, sy = pos[start_state]
theta = start_angle
r = radius_of(start_state)
tail = (sx + (r + 1.0) * math.cos(theta), sy + (r + 1.0) * math.sin(theta))
head = (sx + r * math.cos(theta), sy + r * math.sin(theta))
ax.add_patch(FancyArrowPatch(
    posA=tail, posB=head, arrowstyle="-|>", mutation_scale=18,
    lw=2.2, color="black", zorder=4,
))
ax.text(tail[0] + 0.28 * math.cos(theta), tail[1] + 0.28 * math.sin(theta),
        "start", ha="center", va="center", fontsize=9, color="dimgray")

#plt.show()
plt.savefig("build/graph.png", dpi=300, bbox_inches="tight")
