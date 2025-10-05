ゲーム木を出すもの

おかしいものとその理由
```
違うもの
Node key: -1:010
  Board: 0 1 0 
  Player: White, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 0(1:-110) 2(1:01-1)
子供が両方とも自殺手


Node key: -1:1-11
  Board: 1 -1 1 
  Player: White, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 
これも有り得ない
中央に-1を置かれたところで2には置けないから



Node key: 1:-110
  Board: -1 1 0
  Player: Black, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children:
この状況は有り得ない
->0に-1は置けないから

Node key: 1:0-11
  Board: 0 -1 1
  Player: Black, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 0(-1:1-11)
 
Children: 0(-1:1-11)がおかしい。0には置けない自殺手だから


Node key: 1:01-1
  Board: 0 1 -1
  Player: Black, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children:
この手は置かない。-1が自殺手なのにおいている

Node key: 1:1-10
  Board: 1 -1 0
  Player: Black, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 2(-1:1-11)

Children: 2(-1:1-11)が自殺手
```
一回目の出力
```
Number of nodes explored: 13
Node key: -1:-111
  Board: -1 1 1 
  Player: White, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 
Node key: -1:001
  Board: 0 0 1 
  Player: White, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 0(1:-101) 1(1:0-11) 
Node key: -1:010
  Board: 0 1 0 
  Player: White, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 0(1:-110) 2(1:01-1) 
Node key: -1:1-11
  Board: 1 -1 1 
  Player: White, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 
Node key: -1:100
  Board: 1 0 0 
  Player: White, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 1(1:1-10) 2(1:10-1) 
Node key: -1:11-1
  Board: 1 1 -1
  Player: White, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children:
Node key: 1:-101
  Board: -1 0 1 
  Player: Black, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 1(-1:-111)
Node key: 1:-110
  Board: -1 1 0
  Player: Black, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children:
Node key: 1:0-11
  Board: 0 -1 1
  Player: Black, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 0(-1:1-11)
Node key: 1:000
  Board: 0 0 0
  Player: Black, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 0(-1:100) 1(-1:010) 2(-1:001)
Node key: 1:01-1
  Board: 0 1 -1 
  Player: Black, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children:
Node key: 1:1-10
  Board: 1 -1 0
  Player: Black, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 2(-1:1-11)
Node key: 1:10-1
  Board: 1 0 -1
  Player: Black, Value: UNKNOWN, Outcome: UNKNOWN, Optimal: No
  Children: 1(-1:11-1)

```

