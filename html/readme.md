HTML是标准的一般标记性语言ISO 8879:1986（Standard Generalized Markup Language，SGML）的应用。与html类似的有svg,xml等文档结构，具有非常广泛的应用，所以这个parser写好了，可以在很多地方使用。

## Html文档在C++中的解析结果表示

html中实体皆用BaseElement表示，其中StringElement和TagElement继承BaseElement.StringElement即对象的文字内容，相当于javascript中的对象的innerText（稍微有点不同，javascript中的innerText是tag包含的所有文字内容，但StringElement只表示一段字符串）。TagElement即正常的标签对象，标签一般格式为：
```
<tagName attrName1=attrValue1 attrName2=attrValue2>
a list of TagElements and StringElements, that is a list of BaseElement
</tagName>
```

## Html文档结构说明

这部分主要参考rfc1866.

### tags

TagName是一些固定的名字，如html, head, body, div, img, a, ul, ll等等。且不区分大小写。在开始标志中，TagName必须紧跟着'<',不能有空格。有的单元只有一个开始标志，如断行的`<br>` tag。另外有的tag结尾标志是可选的，如:段落结束`</p>`,列表项`</li>`，定义项`</dt>`,定义描述`</dd>`，parser中遇到相邻并列段落开头，即可认为上一个段落结束了，不用得到段落的结束标志出现。

AttrName和TagName同样不区分大小写。AttrName有一个字母，后面接字母，数字，句点或连词符（有的实现下划线，冒号也支持）。但注意`&amp;`和`&AMP;`是不一样的。AttrName和AttrValue直接通过等号连接，属性只有AttrName，是布尔型的属性，等号两边可以有空格存在。属性值可以为如下情况：

1. 逐字字符。字符通过单引号或双引号引起来，一般render的时候，考虑兼容性，大于号不直接用`>`,而用`&gt;`或`&#62;`表示，其他的字符直接照抄。这种情况是实际中用的最多的情况
2. 一个已命名词。如`<input type=checkbox >`中checkbox。已命名词一般由字母，数字，句点或连词符的序列组成，但是历史原因，有时他允许除了空格和’>'中的任意字符表示（Chrome为了兼容性也是这么做的）。

若要将AttrValue进行render输出，可以考虑将双引号或控制转化为数字字符应用或实体字符引用。

> ```
>             ENTITY      NUMERIC
>         CHARACTER REFERENCE   CHAR REF     CHARACTER DESCRIPTION
>         --------- ----------  -----------  ---------------------
>           HT                  &#9;         Tab
>           LF                  &#10;        Line Feed
>           CR                  &#13;        Carriage Return
>           SP                  &#32;        Space
>           "       &quot;      &#34;        Quotation mark
>           &       &amp;       &#38;        Ampersand
> ```

StringElement的字符组成，对应rfc1866中的Data Characters描述。对于数字字符参照（如`&#60;`），根据ASCII码表变成对应的单字符。数字字符参照中的，分号可以不要，其存在的目的是为了与后面紧接着其他字符区分开，类似latex中大括号显式区分命令。`&`符号只有在后面接`#`或字母的时候才被认为是标识符（markup），否则认为是正常字符。同样的,作为标签的起始和结束标志的大于号小于号，也需要严格满足其规定才会成为一个标识符，否则是正常字符。有的数字字符参照有对应的实体字符参照，如`&#60;`与`&lt;`都表示小于号。
对于不同的Tag,这个字符串的修饰是不一样的，对于div而言，连续多个空格（包括blankspace, tab, newline等）只会渲染为1个空格。但对于pre回车会渲染出换行的效果。不过这个倒不用parser操心，我们全部以原字符保存即可。

parser程序实现中，认为StringElement是紧跟在'>'符号之后的且下一个'<'之前的内容。

> ```
>                   ENTITY      NUMERIC
>         CHARACTER REFERENCE   CHAR REF     CHARACTER DESCRIPTION
>         --------- ----------  -----------  ---------------------
>           &       &amp;       &#38;        Ampersand
>           <       &lt;        &#60;        Less than
>           >       &gt;        &#62;        Greater than
> ```

若是想将一段一般性字符翻译为html中的data characters用于网页渲染或交流，只需要将其中的小于号，大于号和`&`变成对应的实体字符参照或数字字符参照。

有的TagElement是非常强的，如script，只能以`</script>`方式结束，在出现这个结束标志前所有的字符串都是script命令，如下所示

```
<script>Array.prototype.flat&&Array.prototype.includes||document.write('<script src="https://polyfill.io/v3/polyfill.min.js?features=Array.prototype.flat%2Ces6"><\/script>')</script>
```

### Comments

html中，注释以`<!`开始，以`>`结束。其实更准确的说，是包含在两对双连词符内的才是注释，即

```
<! this is not comment -- this is comments -- this is not comment >
```

 不过这里简单处理为`<!`开始到`>`结束的全部为注释，这样的方式还可以将Html Public Text Identifiers给读掉，

```
 <!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
```

实际上Chrome也是这么做的。

## Parser实现

html的词法解析：
html在尖括号内部是一个环境，在尖括号外部是另外一个环境。
前者单词间的空格是可随意添加的，类似于C源码的词法环境，
后者在外部，每个字符都是不可忽略的，参考`pre`环境下展示，但有的标签会对这部分的文字进行修饰，如忽略空格和回车符。

在Chrome中，render一个html，当出现一个tag的开头，读到EOF还没出现tag的结尾，也认为是读到了一个完整的tag，因为tag的结尾符号是已知的。查看Chrome的网页开发者模式，网页的各tag尾部是已经被补充完整。

### Tokenize
html中，单词分几种，
1. 以'<'开头，分三个子类，
   1. 接'!'表示注释
   2. 接'/'表示结束标志
   3. 接正常字母表示开始标签
2. 以字母开头,表示属性的名字，或者属性的值
3. 以'>'开头，表示StringElement对象，这个比较特殊，是游离于各Tag之间的对象。
4. 以单引号或者双引号开始的表示属性的值。

### 具体程序

具体程序如下，在parse之后，html对象中实现了一个简单的render:将文字内容打印。类似`w3m`的dump效果。 

具体程序见`https://github.com/zwlwf/parsers/html`.

## 后话

Html文档，经常不是很规范，所以parser程序的鲁棒性要求比较高。这里parse以尽可能多parse为原则，返回能解析到的最大DOM树。