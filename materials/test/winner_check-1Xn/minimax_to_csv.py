import csv
import re

def parse_minimax_text(filename):
    with open(filename, encoding="utf-8") as f:
        text = f.read()

    # 各ノードを抽出
    node_blocks = re.split(r"-{5,}", text)
    nodes = {}

    for block in node_blocks:
        m = re.search(r"(\d+): \[(Black|White)\] -> W: ([\-0-9]+)", block)
        if not m: 
            continue
        hashval = int(m.group(1))
        player = 1 if m.group(2) == "Black" else -1
        winner = int(m.group(3))

        # 子ノードのハッシュ一覧を抽出
        child_line = re.search(r"Children:\s*(.*)", block)
        children = []
        if child_line:
            raw = child_line.group(1).strip()
            if raw != "(Terminal)":
                parts = raw.split()
                for p in parts:
                    try:
                        children.append(int(p))
                    except:
                        pass

        nodes[hashval] = {
            "player": player,
            "winner": winner,
            "children": children,
        }

    return nodes


def get_root_hash(text):
    # "Board Size:" の後に最初に現れるノードが root
    m = re.search(r"(\d+): \[Black\]", text)
    if m:
        return int(m.group(1))
    return None


def convert_to_csv(minimax_file, csv_file):
    with open(minimax_file, encoding="utf-8") as f:
        text = f.read()

    nodes = parse_minimax_text(minimax_file)

    # 初期局面（黒番）のハッシュ
    root_hash = get_root_hash(text)
    root = nodes[root_hash]

    children = root["children"]

    with open(csv_file, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["move", "color", "winner"])

        for mv, child_hash in enumerate(children):
            w = nodes[child_hash]["winner"]
            if w == 1:
                color = "green"
            elif w == -1:
                color = "red"
            else:
                color = "yellow"

            writer.writerow([mv, color, w])

    print("CSV written:", csv_file)


if __name__ == "__main__":
    convert_to_csv("Minimax1x5.txt", "heatmap_1x5_from_text.csv")
