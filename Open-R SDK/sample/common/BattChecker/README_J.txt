** BattChecker Sample program **

ここでは2つのサンプルプログラムを提供します。いずれのサンプルも
Windows でのみ動作します。

1. BattChecker は、win32api と Remote Processing OPEN-R を使うサンプル
   プログラムです。ホストPC上で簡易な Window を開き、AIBOのバッテリー
   残量を表示します。

2. 2つのプログラムが動作します。1つは Remote Processing OPEN-R を使って
   動作するホストPC上のプログラム(BattChecker)です。もう1つは MFCで作成
   されたWindows のアプリケーション(BattViewer)です。
   2つのプログラムは、Windowsのプロセス間通信（共有メモリー）でデータ
   をやりとりします。
   Remote Processing OPEN-R を使うプログラム(BattChecker)は、AIBOの
   バッテリー残量を取得し、共有メモリーに書き込みます。MFCで作成された
   Windows のアプリケーション(BattViewer)は、共有メモリーからバッテリー
   残量を読み出して、プログレスバーで残量を表示します。
   表示をするWindows アプリケーション(BattViewer)はVisual C++ 6.0 を使って
   作成されています。

■ディレクトリ構成
BattChecker--+--BattChecker--+--host--+--MFC Makefile(host用)
             |               |        |
             |               |        +--win32 Makefile(host用)
             |               |
             |               +--robot Makefile(robot用)
             |
             +--MS (RP OPEN-R を使わない場合; robot用)
             |
             +--RP--+--host--+--MFC--MS  (RP OPEN-Rを使う場合; host用)
             |      |        |
             |      |        +--win32-MS (RP OPEN-Rを使う場合; host用)
             |      |
             |      +--robot--MS (RP OPEN-Rを使う場合; robot用)
             |
             +--BattViewer (MFCアプリ 表示用)

■作成方法・使い方
このサンプルでは3つの方法を提供します。1つ目はAIBOのみで動作します。
2つ目は Win32API を使う簡単なサンプルです。3つ目は Win32API を使う
サンプルとMFCを使うプログラムです。

1. make 時に OPENR_WIN32API と OPENR_SHAREDMEM_FOR_MFCAPP が定義
    されていません。
  RP OPEN-R を使わない場合(AIBOでのみ動作)
  $ cd sample/BattCheker
  $ make install
  $ cd MS
  $ cp -rf OPEN-R <Memory Stick Drive letter>:

2. make 時に OPENR_WIN32API が定義されています。
  RP OPEN-R を使い、win32apiを使ってWindow を表示する場合
  $ cd sample/BattCheker/RP/robot
  $ make install
  $ cd MS
  $ cp -rf OPEN-R <Memory Stick Drive letter>:
  $ cd ../../host/win32
  $ make install
  $ /usr/local/OPEN_R_SDK/RP_OPENR_R/bin/start-rp-openr
  
  start-rp-openr を使う前に、ipc-deamon を動作させておいてください。
  Remote Processing OPEN-Rをセットアップしておく必要があります。
  セットアップには、Installation guide をご覧ください。

3. make 時に OPENR_WIN32API と OPENR_SHAREDMEM_FOR_MFCAPP が定義されています。
  RP OPEN-R を使い、表示をMFCアプリで行う場合
  はじめに Visual C++ で BattViewer をビルドしておく。
  $ cd sample/BattCheker/RP/robot
  $ make install
  $ cd MS
  $ cp -rf OPEN-R <Memory Stick Drive letter>:
  （ロボットにコピーするオブジェクト、設定ファイルは win32用と同じです）
  $ cd ../../host/MFC
  $ make install
  $ /usr/local/OPEN_R_SDK/RP_OPENR_R/bin/start-rp-openr
  
  start-rp-openr を使う前に、ipc-deamon を動作させておいてください。
  Remote Processing OPEN-Rをセットアップしておく必要があります。
  セットアップには、Installation guide をご覧ください。

  BattViewer を VC++を使ってもしくは直接実行する（実行ファイルを
  ダブルクリックする等）

----
"Windows" is a registered trademark of Microsoft Corporation in the 
US and other countries.
