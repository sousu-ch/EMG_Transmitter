EMG Transmitter for M5Stick C Plus
====

筋電信号測定器

## Description
筋電信号をADで取得してLCDにバーグラフ表示出来ます。
取得したデータはリアルタイムにBluetoothのSPPで送信します。
BluetoothつきPC側かM5Stackを用意することで、筋電図表示やデータ記録を行うこともできます。
解説:  http://sousuch.web.fc2.com/DIY/emg/

## Example
![WIREING DIAGRAM](https://github.com/sousu-ch/EMG_Transmitter/blob/master/wiring.png "WIREING DIAGRAM")

## Requirement
M5GFXを使用しています

## Usage
FTDIのシリアル変換モジュールおよび測定対象を、あらかじめUSBに接続した状態で、HID_Latency.exeを起動します。
動作としては下記ような流れになります。

1. センサー及び信号増幅部とM5StickC Pluseを接続し、電源ON
2. 測定部位の最大緊張で飽和しないように、LCDを見ながら信号増幅部の感度を調整する
3. 調整完了したら、ボタンAを押して画面OFF(電池節約)
4. PCか、M5Stackで筋電図表示したり、記録したりする

受信部は別プログラムです
...

## Install
M5GFXをインストールしてから、ビルド書き込みしてください。

## Licence
The source code is licensed MIT.
