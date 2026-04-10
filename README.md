# vilnius-basic-compiler
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/e6afa5b97cf04e169570eca8d9579c04)](https://app.codacy.com/gh/nzeemin/vilnius-basic-compiler/dashboard)
[![CodeFactor](https://www.codefactor.io/repository/github/nzeemin/vilnius-basic-compiler/badge)](https://www.codefactor.io/repository/github/nzeemin/vilnius-basic-compiler)

## Vilnius BASIC Compiler

**BASIC Vilnius** is an implementation of the BASIC programming language for Soviet machines with PDP-11 architecture: DVK, BK, UKNC, Nemiga.
This BASIC is a semi-compiler: it converts the program text into a so-called **threaded code**,
which executes faster than regular interpretation.

The aim of this project is to develop a full-fledged cross-compiler from the BASIC Vilnius language to MACRO assembly code.

The compiler runs on PC (Windows, Linux, macOS) and generates a text file with assembly code (with `.MAC` extension) from a BASIC program, plus a text file `VIBAS.MAC` with runtime assembly code for this program. There is also an option to generate a single common assembly file containing both the main program code and the runtime code.

Current state of the project: **prototype**


## Компилятор Бейсик Вильнюс

**Бейсик Вильнюс** — это реализация языка BASIC для советских машин с архитектурой PDP-11: БК, УКНЦ, Немига.
Этот Бейсик является "полу-компилятором": он преобразует текст программы в так называемый **шитый код**,
который выполняется быстрее, чем при обычной интерпретации.

Данный проект ставит целью написание полноценного кросс-компилятора с языка Бейсик Вильнюс в код для ассемблера MACRO (для УКНЦ) либо BKTurbo8 (для БК-0010).

Компилятор работает на ПК (Windows, Linux, Mac), и из программы на языке BASIC генерирует текстовый файл с ассемблерным кодом (с расширением `.MAC`), плюс текстовый файл `VIBAS.MAC` с ассемблерным кодом рантайма для этой программы. Есть также вариант генерировать один общий ассемблерный файл, содержиащий и код основной программы и рантайм (опция `--onefile`).

Текущее состояние проекта: **прототип**

Ближайшая цель это **версия 0.1**, в которой:
 - подмножество языка с типами Integer и Single, без массивов, строки только константные
 - полноценный парсинг всех конструкций языка, кроме оператора DATA
 - вычисление Integer и Single выражений, вычисление константных String выражений
 - PRINT для Integer/Single и константных строк, INPUT только для целых чисел
 - упрощённый IF: без полноценных операторов под THEN/ELSE, только GOTO
 - функции: PI, ABS, FIX, INT, SGN, CINT, CSNG, PEEK, INP, CSRLIN, POS
 - операторы: LET, GOTO, GOSUB, RETURN, IF/THEN/ELSE (частично), FOR, NEXT, ON, STOP, END, REM, PRINT (включая AT, TAB, SPC, запятая), INPUT, POKE, OUT, CLS, COLOR, LOCATE, BEEP
 - без работы со строками, без работы с файлами, без графики, без DATA/READ/RESTORE
 - НЕ реализованы операторы: `LET MID$`, `IF` с операторами под `THEN`/`ELSE`, `DIM`, `KEY`, `CLEAR`, `DATA`, `READ`, `RESTORE`, `DEF USR`, `DEF FN`, `PRINT` для строковых выражений, `INPUT` для Single и строк, `OPEN`, `CLOSE`, `SCREEN`, `PSET`, `PRESET`, `LINE`, `CIRCLE`, `PAINT`, `DRAW`, `TRON`, `TROFF`, `WIDTH`, `SYSTEM`, `MONIT`

Возможные сценарии использования компилятора:

 1. Для БК-0010. Используя `vibasc`, генерируем ассемблерный файл с опциями: `--platform=BK0010 --turbo8 --onefile`. На выходе получаем один ассемблерный файл, компилируем его на PC ассемблером BKTurbo8, получаем на выходе файл .BIN, который можем замускать в эмуляторе БК или на реальной машине.
 2. Для УКНЦ. В результате компиляции программы на BASIC с опцией `--platfom=UKNC`, получаем .MAC файл основного кода и `VIBAS.MAC` для рантайма. Далее, под операционной системой RT-11, компилируем эти .MAC файлы стандартным для RT-11 ассемблером `MACRO`, получатся объектные файлы с расширением `.OBJ`. Затем, линкуем эти файлы программой `LINK`. В результате, на выходе получается исполнимый файл с расширением `.SAV`, который может быть исполнен в среде RT-11, обычно командой вида `RU MYPROG.SAV`.
 3. Для УКНЦ, кросс-компиляция. То же что и в п.2, но всё делаем на ПК. После получения .MAC файлов используем ассемблер `macro11` и линковщик `pclink11`, полученный .SAV файл запускаем в эмуляторе УКНЦ или на реальной машине.

### Командная строка

`vibasc [опции] filename.ASC [опции]`

где `filename.ASC` — имя файла с программой на BASIC.

Опции:
 - `--quiet`, `-q` — не выдавать строку "о программе" в начале.
 - `--onefile` — на выходе выдавать один файл, содержащий и код основной программы и код рантайма; без этой опции файл рантайма генерится отдельно под именем `VIBAS.MAC`.
 - `--turbo8` — синтаксис выходных файлов должен соответствовать требованиям ассемблера BKTurbo8; как правило, используется для программ под БК, но может применяться и для программ под УКНЦ. Полученный через BKTurbo8 .BIN файл можно сконвертировать в .SAV файл утилитой `BkBin2Sav`. Без указания опции `--turbo8`, синтаксис выходных файлов соответствует ассемблеру MACRO.
 - `--platform={BK0010|UKNC}` — указание целевой платформы, БК-0010 или УКНЦ, по умолчанию `UKNC`; этот параметр влияет на выбор файла с шаблоном рантайма, с названием `runtime-{platform}.tmac`. Файл шаблона рантайма должен находится там же, где и исполнимый файл компилятора.

### Пример

Исходный файл на Бейсике:
```basic
10 A%=23.42
20 PRINT A%
```
Результат компиляции (только основной код, без рантайма):
```assembler
START:
	MTPS	#340		; disable interrupts
	CLR	@#177560
	MTPS	#0		; enable interrupts
	MOV	SP, SAVESP
; 10 A%=23.42
N10:
	MOV	#23., VARIA	; var A% assignment
; 20 PRINT A%
N20:
	MOV	VARIA, R0	; var A%
	CALL	WRINT		; PRINT Integer
	CALL	WREOL
LEND:
SAVESP = . + 2
	MOV	#776, SP	; restore SP
	EMT	350		; .EXIT
; VARIABLES
	.EVEN
VARIA:	.WORD	0	; A%
; RUNTIME CALLS
	.GLOBL	WREOL, WRINT
	.END	START
```

### Особенности этой реализации

Так же, как и в оригинале Бейсик Вильнюс:
 - Один оператор на строку.
 - Имена переменных опознаются по двум первым буквам + тип.

Отличия от оригинала:
 - Ключевые слова нужно писать полностью, сокращения НЕ допускаются.
 - Для величин/переменных вещественного типа есть только тип Single (32 бита, 7 десятичных цифр). Числа двойной точности (например, `1234#` или `235.988D-7`) в тексте программы НЕ распознаются. Нет функции `CDBL`. Значение `PI` используется в точности Single. Все функции с вещественным результатом также отдают тип Single.
 - Номера строк НЕ являются обязательными, они нужны только в случаях, когда на строку нужно сослаться в `GOTO`/`GOSUB`/`ON` и других операторах; исключение: в строке с `FOR` номер нужно указывать.
 - Под оператором `IF` в `THEN`/`ELSE` НЕ допускается ставить операторы `FOR`/`NEXT`/`DIM`/`DATA`
 - Аргумент функций `CSRLIN` и `POS` необязательный, но если есть, то вычисляется (если он не константный), но результат вычисления не используется.
 - `ON .. GOTO/GOSUB`: отрицательное значение выражения под `ON` не приводит к ошибке, просто переходим на строку, следующую после этого оператора.
 - Команды/операторы, которые не реализованы и НЕ БУДУТ реализованы в будущем:
   - `RUN`, `CONT`
   - `KEY` (переназначение функциональных клавиш)
   - `LOAD`, `SAVE`, `CLOAD`, `CSAVE` (загрузка и сохранение текста программы)
   - `LIST`, `MERGE`, `DELETE`, `RENUM`, `AUTO`, `NEW` (работа с текстом программы)
   - `TRON`, `TROFF` (трассировка при выполнении)
   - `SYSTEM`, `MONIT` (выход из Бейсика)
 - Нет пошагового выполнения программы. Вместо этого, можно использовать любой доступный отладчик на уровне готового бинарного кода.

### Декорирование имён переменных

Переменные в Бейсик Вильнюс опознаются по первым двум символам имени + тип.

Имена переменных в ассемблерном коде имеют вид: `VAR` + тип (`I`,`F`,`S`) + первые два символа имени переменной.

Примеры декорирования:
```
A%                    VARIA
B или B!              VARFB
C$                    VARSC
AA или AAA или AA1    VARNAA
```
