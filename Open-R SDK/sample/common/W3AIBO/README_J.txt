
[W3AIBO のビルドと実行]

1. libjpeg の準備

   jpegsrc.v6b.tar.gz を入手
   
   $ ls
   MS        README_E.txt  W3AIBO              makefile.aibo
   Makefile  README_J.txt  jpegsrc.v6b.tar.gz   

2. ビルド

   $ make install

3. 空の AIBO プログラミングメモリスティックに、まず以下の OPEN-R 
   ディレクトリをコピーします。

   ${OPENRSDK_ROOT}/OPEN_R/MS/WCONSOLE/nomemprot/OPEN-R

   次に、以下の OPEN-R ディレクトリを追加上書きします。

   MS/OPEN-R

4. AIBO プログラミングメモリスティックを AIBO に挿入し、AIBO を起動
   します。
   その後の操作（無線コンソール、AIBOのシャットダウン方法）は、従来
   と同じです。

5. Web ブラウザで AIBO にアクセスします。

   Layer H 画像
     http://<AIBO's IP address>:60080/        
     http://<AIBO's IP address>:60080/layerh
     http://<AIBO's IP address>:60080/layerhr (再構成画像)

   Layer M 画像
     http://<AIBO's IP address>:60080/layerm
     http://<AIBO's IP address>:60080/layermr (再構成画像)

   Layer L 画像
     http://<AIBO's IP address>:60080/layerl
     http://<AIBO's IP address>:60080/layerlr (再構成画像)
