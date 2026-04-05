
                                ImageCapture

○概要

  ImageCapture は OPEN-R SDK 付属のサンプルプログラムを組み合わせて作成した
  画像データキャプチャプログラムです。LayerH, LayerM, LayerL の raw データと
  LayerH を BMP 形式に変換したデータをファイルに保存します。

    ※ImageObserver, MoNetTest はソースコードを一部変更しています。


○使用方法

  - 起動後 stand 姿勢になります。その後は動作はしません。

  - あごスイッチ or 背中センサーを押すと /OPEN-R/MW/DATA/P ディレクトリに
    画像データを保存します。

  - データのファイル名
    RGBH0000.BMP - RGBH9999.BMP    LAYER H を BMP 形式に変換したデータ
    LAYH0000.RAW - LAYH9999.RAW    LAYER H raw データ
    LAYM0000.RAW - LAYM9999.RAW    LAYER M raw データ
    LAYL0000.RAW - LAYL9999.RAW    LAYER L raw データ

    ファイルの番号は通しで 0000 - 9999 まで順番につけていきます。
    既に存在するファイルは上書きしません。

  - TinyFTPD を組み込んであるので ftp で画像データを確認することができます。
  
  - 終了する場合はポーズスイッチを押してください。
