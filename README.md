# myshell : 擬似シェル
## 概要
C 言語を用いた擬似シェルプログラムです。
**実用目的での利用は想定しておりません。**

## 利用方法
ディレクトリ内で make コマンドを実行するとコンパイルが実行され、mysh が生成されます。
ターミナル上で mysh を実行 (./mysh) することでシェルの動作を再現します。

## 機能
環境変数からパスを取得し、コマンドを実行します。パイプライン実行にも対応しています。