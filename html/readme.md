HTML�Ǳ�׼��һ����������ISO 8879:1986��Standard Generalized Markup Language��SGML����Ӧ�á���html���Ƶ���svg,xml���ĵ��ṹ�����зǳ��㷺��Ӧ�ã��������parserд���ˣ������ںܶ�ط�ʹ�á�

## Html�ĵ���C++�еĽ��������ʾ

html��ʵ�����BaseElement��ʾ������StringElement��TagElement�̳�BaseElement.StringElement��������������ݣ��൱��javascript�еĶ����innerText����΢�е㲻ͬ��javascript�е�innerText��tag�����������������ݣ���StringElementֻ��ʾһ���ַ�������TagElement�������ı�ǩ���󣬱�ǩһ���ʽΪ��
```
<tagName attrName1=attrValue1 attrName2=attrValue2>
a list of TagElements and StringElements, that is a list of BaseElement
</tagName>
```

## Html�ĵ��ṹ˵��

�ⲿ����Ҫ�ο�rfc1866.

### tags

TagName��һЩ�̶������֣���html, head, body, div, img, a, ul, ll�ȵȡ��Ҳ����ִ�Сд���ڿ�ʼ��־�У�TagName���������'<',�����пո��еĵ�Ԫֻ��һ����ʼ��־������е�`<br>` tag�������е�tag��β��־�ǿ�ѡ�ģ���:�������`</p>`,�б���`</li>`��������`</dt>`,��������`</dd>`��parser���������ڲ��ж��俪ͷ��������Ϊ��һ����������ˣ����õõ�����Ľ�����־���֡�

AttrName��TagNameͬ�������ִ�Сд��AttrName��һ����ĸ���������ĸ�����֣��������ʷ����е�ʵ���»��ߣ�ð��Ҳ֧�֣�����ע��`&amp;`��`&AMP;`�ǲ�һ���ġ�AttrName��AttrValueֱ��ͨ���Ⱥ����ӣ�����ֻ��AttrName���ǲ����͵����ԣ��Ⱥ����߿����пո���ڡ�����ֵ����Ϊ���������

1. �����ַ����ַ�ͨ�������Ż�˫������������һ��render��ʱ�򣬿��Ǽ����ԣ����ںŲ�ֱ����`>`,����`&gt;`��`&#62;`��ʾ���������ַ�ֱ���ճ������������ʵ�����õ��������
2. һ���������ʡ���`<input type=checkbox >`��checkbox����������һ������ĸ�����֣��������ʷ���������ɣ�������ʷԭ����ʱ��������˿ո�͡�>'�е������ַ���ʾ��ChromeΪ�˼�����Ҳ����ô���ģ���

��Ҫ��AttrValue����render��������Կ��ǽ�˫���Ż����ת��Ϊ�����ַ�Ӧ�û�ʵ���ַ����á�

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

StringElement���ַ���ɣ���Ӧrfc1866�е�Data Characters���������������ַ����գ���`&#60;`��������ASCII����ɶ�Ӧ�ĵ��ַ��������ַ������еģ��ֺſ��Բ�Ҫ������ڵ�Ŀ����Ϊ�����������������ַ����ֿ�������latex�д�������ʽ�������`&`����ֻ���ں����`#`����ĸ��ʱ��ű���Ϊ�Ǳ�ʶ����markup����������Ϊ�������ַ���ͬ����,��Ϊ��ǩ����ʼ�ͽ�����־�Ĵ��ں�С�ںţ�Ҳ��Ҫ�ϸ�������涨�Ż��Ϊһ����ʶ���������������ַ����е������ַ������ж�Ӧ��ʵ���ַ����գ���`&#60;`��`&lt;`����ʾС�ںš�
���ڲ�ͬ��Tag,����ַ����������ǲ�һ���ģ�����div���ԣ���������ո񣨰���blankspace, tab, newline�ȣ�ֻ����ȾΪ1���ո񡣵�����pre�س�����Ⱦ�����е�Ч�����������������parser���ģ�����ȫ����ԭ�ַ����漴�ɡ�

parser����ʵ���У���ΪStringElement�ǽ�����'>'����֮�������һ��'<'֮ǰ�����ݡ�

> ```
>                   ENTITY      NUMERIC
>         CHARACTER REFERENCE   CHAR REF     CHARACTER DESCRIPTION
>         --------- ----------  -----------  ---------------------
>           &       &amp;       &#38;        Ampersand
>           <       &lt;        &#60;        Less than
>           >       &gt;        &#62;        Greater than
> ```

�����뽫һ��һ�����ַ�����Ϊhtml�е�data characters������ҳ��Ⱦ������ֻ��Ҫ�����е�С�ںţ����ںź�`&`��ɶ�Ӧ��ʵ���ַ����ջ������ַ����ա�

�е�TagElement�Ƿǳ�ǿ�ģ���script��ֻ����`</script>`��ʽ�������ڳ������������־ǰ���е��ַ�������script���������ʾ

```
<script>Array.prototype.flat&&Array.prototype.includes||document.write('<script src="https://polyfill.io/v3/polyfill.min.js?features=Array.prototype.flat%2Ces6"><\/script>')</script>
```

### Comments

html�У�ע����`<!`��ʼ����`>`��������ʵ��׼ȷ��˵���ǰ���������˫���ʷ��ڵĲ���ע�ͣ���

```
<! this is not comment -- this is comments -- this is not comment >
```

 ��������򵥴���Ϊ`<!`��ʼ��`>`������ȫ��Ϊע�ͣ������ķ�ʽ�����Խ�Html Public Text Identifiers��������

```
 <!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
```

ʵ����ChromeҲ����ô���ġ�

## Parserʵ��

html�Ĵʷ�������
html�ڼ������ڲ���һ���������ڼ������ⲿ������һ��������
ǰ�ߵ��ʼ�Ŀո��ǿ�������ӵģ�������CԴ��Ĵʷ�������
�������ⲿ��ÿ���ַ����ǲ��ɺ��Եģ��ο�`pre`������չʾ�����еı�ǩ����ⲿ�ֵ����ֽ������Σ�����Կո�ͻس�����

��Chrome�У�renderһ��html��������һ��tag�Ŀ�ͷ������EOF��û����tag�Ľ�β��Ҳ��Ϊ�Ƕ�����һ��������tag����Ϊtag�Ľ�β��������֪�ġ��鿴Chrome����ҳ������ģʽ����ҳ�ĸ�tagβ�����Ѿ�������������

### Tokenize
html�У����ʷּ��֣�
1. ��'<'��ͷ�����������࣬
   1. ��'!'��ʾע��
   2. ��'/'��ʾ������־
   3. ��������ĸ��ʾ��ʼ��ǩ
2. ����ĸ��ͷ,��ʾ���Ե����֣��������Ե�ֵ
3. ��'>'��ͷ����ʾStringElement��������Ƚ����⣬�������ڸ�Tag֮��Ķ���
4. �Ե����Ż���˫���ſ�ʼ�ı�ʾ���Ե�ֵ��

### �������

����������£���parse֮��html������ʵ����һ���򵥵�render:���������ݴ�ӡ������`w3m`��dumpЧ���� 

��������`https://github.com/zwlwf/parsers/html`.

## ��

Html�ĵ����������Ǻܹ淶������parser�����³����Ҫ��Ƚϸߡ�����parse�Ծ����ܶ�parseΪԭ�򣬷����ܽ����������DOM����