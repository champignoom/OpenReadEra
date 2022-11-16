#include "include/crcss.h"

const char* CR_CSS_INDENT_NONE = R"delimiter(
body, p, .justindent {
  text-indent: 0em;
}
)delimiter";

const char* CR_CSS_INDENT_NORMAL = R"delimiter(
body, p, .justindent {
    text-indent: 1.2em;
}
)delimiter";

const char* CR_CSS_INDENT_DOUBLE = R"delimiter(
body, p, .justindent {
    text-indent: 2em;
}
)delimiter";

const char* CR_CSS_ALIGN_JUSTIFY = R"delimiter(
body, p, .justindent {
  text-align: justify;
}
)delimiter";

const char* CR_CSS_ALIGN_LEFT = R"delimiter(
body, p, .justindent {
  text-align: left;
}
)delimiter";

const char* CR_CSS_ALIGN_CENTER = R"delimiter(
body, p, .justindent {
  text-align: center;
}
)delimiter";

const char* CR_CSS_ALIGN_RIGHT = R"delimiter(
body, p, .justindent {
  text-align: right;
}
)delimiter";

const char* CR_CSS_BASE = R"delimiter(
.empty-line, empty-line {
  display: block;
  height: 1em;
}

hr {
  display: block;
  height: 2px;
  background-color: #808080;
  margin-top: 0.5em;
  margin-bottom: 0.5em;
}

body, p, .justindent {
  display: block;
  margin-top: 0em;

  margin-left: 0em;
  margin-right: 0em;
}

DocFragment {
  page-break-before: always;
}

/* TEST AREA */

/* TODO */

img, image, .section_image, .coverpage, coverpage {
  display: block;
  text-align: center;
  text-indent: 0px;
  margin-top: 0em;
  margin-bottom: 0em;
  margin-left: 0em;
  margin-right: 0em;
  padding-top: 1em;
  padding-bottom: 1em;
}

video {
  display: block;
  page-break-before: avoid;
  text-align: center;
  text-indent: 0px;
  margin-top: 0em;
  margin-bottom: 0em;
  margin-left: 0em;
  margin-right: 0em;
  padding-top: 1em;
  padding-bottom: 1em;
}

p image, li image , p img, li img {
  display: inline;
  padding-left: 1em;
  padding-right: 1em;
}

/* for that inline images in cites just putting them to block */
cite p image{
  display: block;
}

a, b, strong, q, u, del, s, strike, small, big,
sub, sup, acronym, span, font, abbr, time, li p {
  display: inline;
}

hidden, .hidden {
  display: none;
}


b, strong {
  font-weight: bold;
}
i, em, emphasis, dfn, var {
  display: inline;
  font-style: italic;
}
u {
  text-decoration: underline;
}
/* Mozilla: 0000EE Dark: 00569f Light: 4285f4 Lebedev: 008ACE */
.link_valid {
  color: #00569f;
}
del, s, strike, strikethrough {
  text-decoration: line-through;
}
small {
  font-size: 80%;
}
big {
  font-size: 120%;
}
sub {
  vertical-align: sub;
  font-size: 70%;
}
sup {
  vertical-align: super;
  font-size: 70%;
}
sup small{
  vertical-align: super;
  font-size: 100%;  /* 100% of sup == 70% of regular text*/
}
nobr {
  display: inline;
  hyphenate: none;
  white-space: nowrap;
}

h1, title, .title, .title0, .title1 {
  font-size: 140%;
}
h2, .title2 {
  font-size: 130%;
}
h3, .title3 {
  font-size: 130%;
}

h1, h2, h3, title{
  page-break-before: always;
  page-break-inside: avoid;
  page-break-after: avoid;
}

h1, h2, h3, title,
h1 > p, h2 > p, h3 > p, title > p,
.title, .title0, .title1, .title2, .title3,
.title > p, .title0 > p, .title1 > p, .title2 > p, .title3 > p
h1 > span, h2 > span, h3 > span, title > span{
  display: block;
  hyphenate: none;
  adobe-hyphenate: none;
  text-align: center;
  text-indent: 0em;
  margin-top: 0.4em;
  margin-bottom: 0.4em;
  margin-left: 0em;
  margin-right: 0em;
  font-weight: bold;
}
h4, h5, h6, subtitle,
h4 > p, h5 > p, h6 > p, subtitle > p,
.subtitle, .title4, .title5, .title6 {
  display: block;
  font-weight: bold;
  page-break-inside: avoid;
  page-break-after: avoid;
  hyphenate: none;
  adobe-hyphenate: none;
  font-size: 110%;
  text-align: center;
  text-indent: 0em;
  margin-top: 0.3em;
  margin-bottom: 0.3em;
  margin-left: 0em;
  margin-right: 0em;
  font-style: italic;
}

pre, code, .code {
  font-size: 80%;
  display: inline;
  white-space: pre;
  text-align: left;
  text-indent: 0em;
  margin-top: 0em;
  margin-bottom: 0em;
  margin-left: 0em;
  margin-right: 0em;
  font-family: "Roboto Mono", "Droid Sans Mono", monospace;
}

p.code{
  display: block;
}

tt, samp, kbd {
  display: inline;
  font-family: "Roboto Mono", "Droid Sans Mono", monospace;
}

blockquote {
  display: block;
/*  hyphenate: none;*/
  font-style: italic;
  margin-left: 1.5em;
  margin-right: 1.5em;
  margin-top: 0.5em;
  margin-bottom: 0.5em;
}

cite{
  display: inline;
  font-style: italic;
}

.citation > p, cite > p {
  display: block;
  text-align: justify;
  text-indent: 1.2em;
  margin-top: 0.3em;
  margin-bottom: 0.3em;
  margin-left: 1em;
  margin-right: 1em;
  font-style: italic;
}

ol, ul , list{
  display: block;
  margin-top: 1em;
  margin-bottom: 1em;
  margin-left: 0em;
  margin-right: 0em;
  padding-left: 20px;
}
ol {
  list-style-type: decimal;
}
ul,list {
  list-style-type: disc;
}
li {
  display: list-item;
  text-indent: 0em;
}

dl {
  display: block;
  margin-top: 1em;
  margin-bottom: 1em;
  margin-left: 0em;
  margin-right: 0em;
}
dt {
  display: block;
  margin-left: 0em;
  margin-top: 0.3em;
  font-weight: bold;
}
dd {
  display: block;
  margin-left: 1.3em;
}

table {
  font-size: 80%;
}
td, th {
  text-indent: 0px;
  padding: 3px;
}
td > p, td p {
text-indent: 0px;
text-align: center;
}
th {
  font-weight: bold;
  text-align: center;
}
table > caption {
  text-indent: 0px;
  padding: 4px;
}

v, .v {
  text-align: left;
  text-align-last: right;
  text-indent: 1em hanging;
}

/*.stanza + .stanza,*/
/* {.class + .class} selector does not exist*/

 stanza + stanza {
  margin-top: 1em;
}

.stanza, stanza {
  text-align: left;
  text-indent: 0em;
  margin-top: 0.3em;
  margin-bottom: 0.3em;
  margin-left: 15%;
  margin-right: 1em;
  font-style: italic;
}

.poem, poem {
  margin-top: 1em;
  margin-bottom: 1em;
  text-indent: 0px;
}

.epigraph > p, epigraph, epigraph > p {
  text-align: right;
  text-indent: 1em;
  margin-top: 0.3em;
  margin-bottom: 0.3em;
  margin-left: 15%;
  margin-right: 1em;
  font-style: italic;
}

text-author, .epigraph_author, .citation_author {
  font-size: 80%;
  font-style: italic;
  text-align: right;
  margin-left: 15%;
  margin-right: 1em;
}


NoteFragment title ,
body[name="notes"] > title ,
body[name="notes_hidden"] > title {
  page-break-before: avoid;
  page-break-inside: avoid;
  page-break-after : avoid;
}

section li {
  page-break-before: avoid;
}

body li {
  page-break-before: avoid;
  page-break-inside: avoid;
  page-break-after : avoid;
}

body[name="notes"] > section {
  margin-bottom: 0.5em;
}

body[name="notes"] > section title,
body[name="notes"] > section title > a ,
body[name="notes_hidden"] > section title,
body[name="comments"] > section > title {
  font-size: 100%;
  display: run-in;
  text-align: left;
  page-break-before: avoid;
  page-break-inside: avoid;
  page-break-after : avoid;
}

table title,tr title,td title,
table h1,tr h1,td h1,th h1,
table h2,tr h2,td h2,th h2,
table h3,tr h3,td h3,th h3,
table h4,tr h4,td h4,th h4,
table h5,tr h5,td h5,th h5,
table h6,tr h6,td h6,th h6
{
  font-size: 100%;
  page-break-before: avoid;
  page-break-inside: avoid;
  page-break-after : avoid;
}

body[name="notes_hidden"] {
  font-size: 70%;
}

body[name="comments"] > section > title > p ,
body[name="notes"] > section > title > p {
  display: inline;
}

a.note_class {
  color: #00569f;
  vertical-align: super;
  font-size: 70%;
  text-decoration: none;
  font-style: normal;
}

a[type="note"] sup {
  vertical-align: super;
  font-size: 70%; /* 100% of sup == 70% of regular text ( a[type="note"] )*/
  text-decoration: none;
}

sup a[type="note"] {
  vertical-align: super;
  font-size: 100%; /* 100% of a[type="note"] == 70% of regular text (sup)*/
  text-decoration: none;
}

p[dir="rtl"], html[dir="rtl"], body[dir="rtl"], div[dir="rtl"], .rtl  {
  display: block;
  hyphenate: none;
}

.center{
text-align: center;
}

.left{
text-align: left;
}

.right{
text-align: right;
}

.justify{
text-align: justify;
}

annotation {
  display: block;
  font-size: 80%;
  margin-left: 1em;
  margin-right: 1em;
  font-style: italic;
  text-align: justify;
  text-indent: 1.2em;
}

.fb2_info {
  display: block;
  page-break-before: always;
}

description, title-info {
  display: block;
}

date {
  display: block;
  font-style: italic;
  text-align: center;
}

genre, author, book-title, keywords, lang, src-lang, translator {
  display: none;
}
document-info, publish-info, custom-info {
  display: none;
}

head, form, script {
  display: none;
}

pagebreak{
  page-break-after: always;
  height: 0;
}


ruby {
    display: inline;
}

ruby > rp
{
    display: none;
}

ruby > rt  {
  display: none;
  vertical-align: middle;
  font-size: 50%;
}

)delimiter";