# Kanji14 形式

Unicode の文字コード U+4E03 〜 U+8E0F でバイト列を符号化した形式です。

Unicode では、U+4E03 〜 U+8E0F は全て CJKV Unified Ideographs に含まれています。この範囲の文字コードは、全て表示される文字(漢字)が割り当てられています。<br>(参考ツール：[Unicode Viewer](https://github.com/ikiuo/dhtml-unicode-viewer))

符号化に用いる文字には区切り記号の類や割り当てのない文字コードがないため、単純な処理で符号化と復号が可能です。

## 符号化処理の概要

1. バイト列を 14 ビット単位で分割する(不足分は 0 を追加する)
1. 14 ビットの値に 0x4E10 を加えた Unicode の文字コードにする
1. 不足分を追加した場合は、以下の文字コードを追加する
   - 文字コード : 0x4E10 - (14 - 追加ビット数)

## 復号処理の概要

1. 文字コードから 0x4E10 を引いて 14 ビットのビット列を作る
   - U+4E03 〜 0x8E0F 以外は無視
1. 文字コードが U+4E03 〜 U+4E0F の場合
   - 最終 14 ビット中の有効ビット数を示す
     - 有効ビット数 ＝ 0x4E10 − 文字コード
1. バイト（8 ビット単位）列に変換する

## 注意点

符号化された文字列は UTF-8/16/32 で保存されることを想定しています。他の文字コード体系(例: Windows で使用されるシフトJIS など)への変換は出来ないでしょう。


# バイト列の扱い

バイト列をビット列として扱う手法を示します。

## 1 バイトのビット番号

8 ビット データである 1 バイト中では、ビット番号の最上位ビットを 0 とし、最下位ビットを 7 とします。

|ビット位置|0|1|2|3|4|5|6|7|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|係数|128|64|32|16|8|4|2|1|

## バイト列とビット番号

1 バイト中のビット位置(0〜7)を B として、バイト位置を N とすると、先頭からのビット位置 P は以下とします。
<blockquote>
P ＝ N × 8 ＋ B
</blockquote>

|バイト位置 N|0|1|2|...|
|:-:|:-:|:-:|:-:|:-:|
|ビット位置 P|0..7|8..15|16..23|...|

# 符号化

以下の手順で符号化します

1. バイト列を 14 ビット単位で分割する
1. 区切った 14 ビット値に 0x4E10 を加える
1. バイト列の終わりまで上記を繰り返す
1. バイト列の長さが 7 の倍数でない場合は「終端コード」を追加する

## バイト列の 14 ビット区切り

符号化する 14 ビット区切りのデータ順と先頭からのビット位置の関係は以下とします。

|14ビット順|0|1|2|3|...|
|:-:|:-:|:-:|:-:|:-:|:-:|
|ビット位置|0..13|14..27|28...41|42..55|...|


### 情報文字

区切った 14 ビット値は 0x4E10 を加えた文字コードを**「情報文字」**とします。

- 情報文字：文字コード = 14ビット値 + 0x4E10

## バイト列の長さが 7 の倍数の場合

全て情報文字で構成されます。

## バイト列の長さが 7 の倍数でない場合

バイト列の長さが 7 の倍数でないとき、14 ビット単位に分割すると最後の有効なビット数は 14 未満になります。そのため、残りに 0 を追加する必要があります。

### 0 を追加するビット数

バイト列の長さを 7 で割ったときの余りを剰余バイト数とします。14ビット値のうち対応する有効ビット数と追加ビット数は以下になります。

- 剰余バイト数 ＝ バイト列の長さ ％ 7
- 有効ビット数 ＝ (剰余バイト数 × 8) % 14
- 追加ビット数 ＝ 14 − 有効ビット数

|剰余バイト数|1|2|3|4|5|6|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|有効ビット数|8|2|10|4|12|6|
|追加ビット数|6|12|4|10|2|8|

### 制御文字

有効なビット数を示す文字コードを**「制御文字」**とし以下の文字コードとします。制御文字はデータの終わりを示すとともに、バイト列の復号時に使用されます。

- 制御文字：文字コード ＝ 0x4E10 − 有効ビット数


# 復号

復号では、文字コードの範囲 U+4E03 〜 U+8E0F を対象とし、それ以外の文字コードは無視します。

## 情報文字

情報文字の文字コードから 0x4E10 を引いて 14ビットのデータとすると、情報文字の文字列から 14 ビットのデータ列が生成できます。

情報文字に対応するビット位置は以下になります。

|情報文字位置|0|1|2|3|...|
|:-:|:-:|:-:|:-:|:-:|:-:|
|ビット位置|0..13|14..27|28...41|42..55|...|

これを 8 ビット単位に区切ると、バイト列として復号できます。

### バイト列の最大長

情報文字数とバイト列の最大長の関係は以下になります。

- バイト列の最大長 = ⌊(情報文字数 × 14) ÷ 8⌋
  - <small> ⌊x⌋ は床関数 floor(x) です</small>

バイト列の「長さ」ではなく「最大長」です。制御文字次第でバイト列の長さは、最大長より短くなります。

## 制御文字がない場合

バイト列の長さは、バイト列の最大長と同じです。

## 制御文字がある場合

制御文字 U+4E03 〜 U+4E0F はデータの終わりとともに、最後の 14 ビット中で先頭から有効なビット数を示しています。

- 有効ビット数 ＝ U+4E10 - 制御文字コード
- 追加ビット数 ＝ 14 - 有効ビット数

### バイト列の長さ

追加ビット数が 8 未満の場合は「バイト列の長さ」は「最大長」と同じですが、8 以上の場合は、最大長から -1 します。

# 実行例

Python プログラム kanji14.py を bzip2 圧縮したバイナリをを Kanji14 形式にする。

```
$ bzip2 -c kanji14.py | python3 kanji14.py -e -w 40
度璓狕佩垤莡纱巢丐機谑剤帐咯踍薷嚁滻踏渎我微篙竁艹砩笺癙倠璥帀碉獆潣譣沤譓茮狒儳
弅巤懤沪幺沘瑝硝培俵呖湄丐耐丐悮皠坟坍懤譝孁俺兝侶璓慖噸丞昒娣刓丶偸耪向娡灱嚡廘
朹襖炊梷矹媠侰世澰从丰裄棣碆薝笃崻痮惱僋尨褾檊梷儩炚调囄嵗臀戕眷耈朶煃剣夲蒣瑹步
牡媑瞶耦眴亖園语猋凔珘屡誚偟慏蓻冖菆槖贪箇珪菌盚凥皮呻溲艩虤襷桼攲贌煉珳唓縁畞簚
细聗洞艦蒲妛婈敳緑覙害匥笃讨敺攒照墭沔櫡烺嚓伤渔庛蓐澓椉凒蘿蓮掕幃幈溚玥癧珴瀽塐
玃懒級碘梜襡弝湗県汯曛杳浃嚢姟屩壽岷詬汮蚕箣岄衖璕经婁攬恴湲晋煈衝绞幹菋赮蓵箧詥
歧藖彶熐媙烂朜割瀰堮蠞砹憢瀓囆墵簜橯摷亙柡吺刴嶤肒爼垢卄燃濮甓愹眊葸罹坨粍満瓐堓
囵腪盤牲琰羖碃螲圓蚯梒糲磍楉呪佡孁俋毈繹睔憧峤榡縍趎洳貹拽坂愮晸匓窱藬擅侌犷磲擦
冮圄牯篩皁杬蕤籪貢藢缃徶熮袣甲妻粦妊意挱湱唕貓溋氟褵堙夣拣検啱恦河瑣缓掀杜缘筊葭
盒檧排燰藗旱虹暇掆每懭衺椼裵喰侖棺冷帢憛噬臉妔糕楩敄砥譴燎菓訑烒佫礟擐恒璈斐性稥
融儡跧赊昨倀神舠携交奋批昦冟壠蕘瞝談溸炸罧耉喹懽查翀憺呲莻揬愤蚃誢卡緆梏蝆伒緂葃
掖盍庈烖貞纯皉晘璴巻旪嵘胡狭塺田弶害淡墕帱蚐纡倦裮懘場棅吺経誰膍糷愠揲珛奰烴力芻
漫択葁稫榉憔僒纻囖揼篥禩嵍覄湸恔炡妯亝暰摣疸粉蟑厃犵峂绬婣搚暖桥褉耪匑艬斦濭袲艤
螚俨诨艪挴竖正涀區眷耦獒竕哽諂朢渜恕健婔漗豶狃倱壓玉娡栰缨燴犼攍裁娒蘾琞讐怘烬坊
惐哐晔壺誦譧罵帶橐羍甅莚著敹翠絘蟽嶰岆涁犢拈章啧媆笚睳褊棿傻誆硃壶堰圓渞別玅揮虾
臄貱獕康尗蟴觏侣裔燛嬯撝桧懶耥胏卒厳烰缣桂圓婙蘙稜澡岂娚氨堫怰荼溿堫腲嬛剷榱宖绩
萳敵啭紜螘櫤螁吒滙乑湂堳姞椣哲偘臨森峗犽橚濶呐崽柋棄今捒惢玁煩筪罸滮珫籕杧脺岾嫽
腇嬣槯咣吵揽仑萛蠪牤縼萵缥蓣婷尗货檡憕偐樸戄渐丌
```

復号してソース コードを確認する

```
$ bzip2 -c kanji14.py | python3 kanji14.py -e | python3 kanji14.py -d | bzip2 -cd
~~~ 結果は省略 ~~~
```
