# Maestro 6軸ロボットアーム　(引き継ぎ資料)

![](/docs/img/green-arm.jpg)
6軸ロボットアームの動作環境周りの資料です


## 内容、フォルダ構成

- docs : ドキュメントおよびチュートリアル
  - [/docs/1-全体構造.md](/docs/1-全体構造.md)
  - [/docs/2-環境構築.md](/docs/2-環境構築.md) モータードライバの設定、ビルド用の環境構築
  - [/docs/3-動作確認.md](/docs/3-動作確認.md)　プログラムの動作
- model : ロボットアームの3Dモデル
  - 手で実機を測定し、Solidworksで3DCADに書き起こしたものなのでサイズが異なっている可能性があります。
  - 詳細は先方からいただいたPythonの`green_core/config/config.yaml`を見ると、各関節の長さや慣性行列など各種パラメータを取得できます
  - URDFモデルを使うと(`/model/robot.urdf`)、各関節を動かしながらビジュアライズできます
- src : プログラム
  - `/src/pc`: PC側で実行するプログラムとサンプルファイル
  - `/src/maestro` : Maestro側で実行するプログラム
  - `/src/common`: MaestroとPCのぷろぐらむで共通する部分。PC↔︎MaestroのModbus通信フォーマットなどを定義している
- `.github/workflows` : GithubのCIテスト。プログラムの動作確認を行う
- `reference`　公開できそうな参考データやデータシートの配布


## faq

### [Elmo SSH中] ElmoにSSHできない

- SSH時に`Unable to negotiate with 192.168.2.52 port 22: no matching host key type found. Their offer: ssh-rsa`って言われる
- `ssh -oHostKeyAlgorithms=+ssh-rsa -oPubkeyAcceptedAlgorithms=+ssh-rsa user@192.168.2.52`


### [Export Tuning中] Drive Error: TC=0.176 Servo (SO) must be on

サーボがONになってないエラーだが、エンコーダの設定がおかしいときもこのエラーが出る。

モータを動かそうとしてエンコーダで実際の動作を読み取りチューニングする際に表示された

### [Export Tuning中] Speed Error limit exceeded refer to : ER[2]

アブソリュートエンコーダの場合、エンコーダ設定のTotal Bitsが間違っていないかを確認する

ちなみにER[2]とはExpert TunigのVelocity & Positionチューニングの項目のパラメータ

### トルク制御したくて目標トルク指令を送ると、Axis operation Mode is different!!って言われる。

- Maestro Application Studioで`EtherCAT Configuration`の目標トルク指令を送信するように設定
- それでもうまくいかない時は再起動する

### Maestroが以下のえらーで落ちる
```sh
torque control poweron
power on start......a01
[2023-10-04 13:21:59.406] [error] CMMCException: MMC_MoveTorqueCmd:
[2023-10-04 13:21:59.406] [error]    : axisref = 0
[2023-10-04 13:21:59.406] [error]    : error = -8
[2023-10-04 13:21:59.406] [error]    : status = 16
[2023-10-04 13:21:59.406] [error] Node state is unsuitable for the active function block
excp.what() = MMC_MoveTorqueCmd:
excp.axisRef() = 0
excp.error() = -8
excp.status() = 16
closing modbus server
```

電源をONにして最初の起動でこのエラーが出ることがある。

Maestro側のプログラムをCtrl+Cでキルして再起動するとうまくいくことが多い。それでも無理なら電源ごと再起動


### Maestroが以下のえらーで落ちる
```sh
.-----------------------------------------
----     Get Error\Warning           ----
----     From API function           ----
-----------------------------------------
Function name:  MMC_OpenUdpChannelCmd:
Return value :  -1
```

- 他にMaestro内部でモータドライバと通信しているプロセスがないか確認
- Maestro上でドライバに命令を送ったのに，それを終了させずに重複して命令を送ってますよ的なエラー
- Maestroのコンセントをぶち抜いて再起動するしかない
- もしくは、`Elmo Application Studio`で`System Configurations`タブを選択 -> `Reset`で矯正リセット


### Elmoに繋がらない！！！

この問題が本当に大変でした、、、、. というのも、Elmoの仕様が複雑で自身を固定IPだったデフォルトゲートウェイの設定が色々あったりで、、、

-> [/docs/2-環境構築.mdの](/docs/2-環境構築.md)
