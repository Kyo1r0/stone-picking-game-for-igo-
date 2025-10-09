import json
import sys
from graphviz import Digraph


def visualize_game_tree(json_path, output_path="gametree.png"):
    # JSONファイル読み込み
    with open(json_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    nodes = data.get("nodes", {})
    dot = Digraph(comment="Game Tree Visualization", format="png")

    # outcome_class に応じた色設定
    color_map = {
        "P": "lightgreen",  # Previous player win
        "N": "orange",      # Next player win
        "L": "lightblue",   # Left player win
        "R": "lightcoral",  # Right player win
        "UNKNOWN": "white",
        "": "white"
    }

    # ノード描画
    for node_key, node_data in nodes.items():
        board = node_data.get("board_state", [])
        value = node_data.get("game_value", "UNKNOWN")
        outcome = node_data.get("outcome_class", "UNKNOWN")
        label = f"{node_key}\n{board}\nV={value}, O={outcome}"

        dot.node(
            node_key,
            label=label,
            style="filled",
            fillcolor=color_map.get(outcome, "white"),
            shape="box",
            fontsize="10"
        )

    # エッジ描画
    for node_key, node_data in nodes.items():
        children = node_data.get("children", {})
        for move, child_key in children.items():
            is_optimal = False
            # 子ノードが最善手なら太線で描く
            if child_key in nodes and nodes[child_key].get("is_optimal", False):
                is_optimal = True
            dot.edge(
                node_key,
                child_key,
                label=f"move {move}",
                penwidth="2" if is_optimal else "1",
                color="black"
            )

    # 出力
    dot.render(filename=output_path, cleanup=True)
    print(f"[OK] ゲーム木画像を出力しました: {output_path}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python visualize_tree.py <analysis_1x3_m1.json>")
        sys.exit(1)

    json_file = sys.argv[1]
    visualize_game_tree(json_file)
