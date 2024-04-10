
# 全体構成（モジュール）

```mermaid
graph TD;

subgraph Robot
r_motor[[モータ]]
r_sensor[[センサ]]
end

subgraph Simulator
s_motor[[モータ]]
s_sensor[[センサ]]
end

modbus[(modbus サーバー)]
r_motor --> modbus
r_sensor --> modbus
s_motor --> modbus
s_sensor --> modbus

subgraph Controller
教師データ生成>教師データ記録]
動かす>動かす]
学習モジュール
database[(教師データ)]
end

学習モジュール --学習モデル-->database 
教師データ生成 --> modbus
教師データ生成 --> database
動かす --> modbus
```

# タスク

- [ ] シミュレーション
  - [ ] モデル作成
  - [ ] ROSにURDF読み込み
  - [ ] かんたんな制御を試す（PIDとか）
  - [ ] モデルを作成
  - [ ] modbusから受け取った値で制御する
- [ ] 実機動作
  - [ ] モータ１個の動作確認
  - [ ] modbus環境構築→モータを動かす
  - [ ] 姿勢角度と電流値などのセンサ情報を読み取る
  - [ ] デバイスチェックとかのコードを書く
  - [ ] modbus経由でモータ制御
  - [ ] 電流制御
  - [ ] ハード制作
    - [ ] ハンド設計
    - [ ] ハンド制作
    - [ ] 回路取り替え
    - [ ] モータ制御
  - [ ] アーム全体でmodbusで動かす
  - [ ] 2台のロボットをシンクロ動作させる
- [ ] コントローラ制作
  - [ ] 回路設計
  - [ ] modbusで通信する 
- [ ] 学習手法の確率
  - [ ] 論文読む
  - [ ] 実装

