
			SoccerLion for ERS-210/220


[ディレクトリ]

  SoccerLion200     --+-- OMWares --+-- include  ヘッダファイル
                      |             +-- lib      ライブラリ
                      |             +-- MS       MW オブジェクト (バイナリ)
                      |
                      +-- SoccerLion                    
                      |
                      +-- PowerMonitor
                      |
                      +-- MotionCommander  (ERS-210/210A でのみ動作)


[サンプルプログラム SoccerLion のビルドと実行]

  1. ビルド

     $ cd SoccerLion
     $ make install

  2. 空の AIBO プログラミングメモリスティックに、まず以下の OPEN-R 
     ディレクトリをコピーします。

     /usr/local/OPEN_R_SDK/OPEN_R/MS/WCONSOLE/nomemprot/OPEN-R

     次に、以下の OPEN-R ディレクトリを追加上書きします。

     SoccerLion/MS/OPEN-R

    * SoccerLion は nomemprot の環境でのみ動作します。

  3. AIBO プログラミングメモリスティックを AIBO に挿入し、AIBO を起動
     します。
     その後の操作（無線コンソール、AIBOのシャットダウン方法）は、従来
     と同じです。

  4. AIBO が立ち上がった後、頭センサーを押してください。AIBO はピンク
     ボールを探し始めます。ボールを発見すると近付いて、ドリブルするか、
     ボールを蹴ります。

[サンプルプログラム MotionCommander のビルドと実行]

  1. ビルド

     $ cd MotionCommander
     $ make install

  2. 空の AIBO プログラミングメモリスティックに、まず以下の OPEN-R 
     ディレクトリをコピーします。

     /usr/local/OPEN_R_SDK/OPEN_R/MS/WCONSOLE/nomemprot/OPEN-R

     次に、以下の OPEN-R ディレクトリを追加上書きします。

     MotionCommander/MS/OPEN-R

    * MotionCommander は nomemprot の環境でのみ動作します。
    * MotionCommander は ERS-210/210A でのみ動作します。

  3. AIBO プログラミングメモリスティックを AIBO に挿入し、AIBO を起動
     します。その後は、無線コンソールを使ってAIBOに命令します。

  4. 無線コンソールに MotionCommander> が出たらAIBOを無線でコントロールできます。
     利用できる命令一覧は '?' の入力で見ることができます。
     
  5. AIBOの終了には 'q' と入力してください。
