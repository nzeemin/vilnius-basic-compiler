# vilnius-basic-compiler
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/e6afa5b97cf04e169570eca8d9579c04)](https://app.codacy.com/gh/nzeemin/vilnius-basic-compiler/dashboard)
[![CodeFactor](https://www.codefactor.io/repository/github/nzeemin/vilnius-basic-compiler/badge)](https://www.codefactor.io/repository/github/nzeemin/vilnius-basic-compiler)

## Vilnius BASIC Compiler

**BASIC Vilnius** is an implementation of the BASIC programming language for Soviet machines with PDP-11 architecture: DVK, BK, UKNC, Nemiga.
This BASIC is a semi-compiler: it converts the program text into a so-called **threaded code**,
which executes faster than regular interpretation.

The aim of this project is to develop a full-fledged cross-compiler from the BASIC Vilnius language to `MACRO` assembly code.
In other words, the compiler runs on a PC (Windows, Linux, Mac) and generates a text file with the `.MAC` extension.
Then, under the **RT-11** operating system, the `.MAC` file is compiled using the standard RT-11 MACRO assembler,
resulting in an object file with the `.OBJ` extension. The object file is then linked with other object modules using the `LINK` program,
including the language's runtime, and optionally, custom assembly procedures.
As a result, an executable file with the `.SAV` extension is produced, which can be executed under the RT-11
on the target machine.

Current state of the project: **prototype**

## Компилятор Бейсик Вильнюс

**Бейсик Вильнюс** — это реализация языка BASIC для советских машин с архитектурой PDP-11: ДВК, БК, УКНЦ, Немига.
Этот Бейсик является полу-компилятором: он преобразует текст программы в так называемый **шитый код**,
который выполняется быстрее, чем при обычной интерпретации.

Данный проект ставит целью написание полноценного кросс-компилятора с языка Бейсик Вильнюс в код для ассемблера `MACRO`.
То есть, компилятор работает на PC (Windows, Linux, Mac), генерирует текстовый файл с расширением `.MAC`.
Далее, под операционной системой **RT-11**, `.MAC` файл компилируется стандартным для RT-11 ассемблером MACRO,
получается объектный файл с расширением `.OBJ`. Затем объектный файл линкуется программой `LINK` с другими
объектными модулями — это runtime языка, плюс, если нужно, свои собственные процедуры на ассемблере.
В результате, на выходе получается исполнимый файл с расширением `.SAV`, который может быть исполнен в среде RT-11
на целевой машине.

Текущее состояние проекта: **прототип**

### Пример

Исходный файл на Бейсике:
```
10 A%=23.42
20 PRINT A%
```
Результат компиляции:
```
	.MCALL	.EXIT
START:
; 10 A%=23.42
L10:
	MOV	#23., VARA.I	; assignment
; 20 PRINT A%
L20:
	MOV	VARIA, R0
	CALL	WRINT
	CALL	WRCRLF
L65536:
	.EXIT
; VARIABLES
VARIA:	.WORD	0	; A%
	.END	START
```

### Декорирование имён переменных

Переменные в Бейсик Вильнюс опознаются по первым двум символам имени + тип.

Имена переменных в коде для MACRO имеют вид: `VAR` + тип (`I`,`N`,`S`) + первые два символа имени переменной.

Примеры декорирования:
```
A%                    VARIA
B или B!              VARNB
C$                    VARSC
AA или AAA или AA1    VARNAA
```
