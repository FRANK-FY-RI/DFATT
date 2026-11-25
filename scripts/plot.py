import networkx as nx
import matplotlib.pyplot as plt

N = int(input())
M = int(input())

Symbols = [int(value) for value in input().split()]

with open("data/symbol_names.tsv", encoding="ascii") as symbol_file:
    SYMBOL_NAMES = [None] * 256
    for line in symbol_file:
        code, name = line.rstrip("\n").split("\t")
        SYMBOL_NAMES[int(code)] = name

edges = []

for i in range(N):
    temp = input().split()
    u = temp[0]

    for j in range(M):
        edges.append((u, temp[j+1], SYMBOL_NAMES[Symbols[j]]))


#print(edges)

G = nx.DiGraph()

# Add edges with symbols as labels instead of weights
for u, v, symbol in edges:
    if G.has_edge(u, v):
        G[u][v]["label"] += "," + symbol
    else:
        G.add_edge(u, v, label=symbol)

# Calculate vertex positions
pos = nx.spring_layout(G, seed=42)

# Draw vertices and directed edges
nx.draw(
    G,
    pos,
    with_labels=True,
    node_color="skyblue",
    node_size=1200,
    arrows=True,
    arrowsize=20,
    edge_color="gray",
    width=1.5
)

# Draw edge labels
edge_labels = nx.get_edge_attributes(G, "label")

nx.draw_networkx_edge_labels(
    G,
    pos,
    edge_labels=edge_labels,
    font_size=10
)

#plt.show()
plt.savefig("build/graph.png", dpi=300, bbox_inches="tight")
