# PlanetExplosionSimulator (OpenGL)

Анимированное графическое приложение, демонстрирующее взрыв планеты и физику разлета осколков с использованием OpenGL. Управляется в реальном времени клавишами, поддерживает освещение, тени, текстуры и движение объектов с различной динамикой.

## ✨ Функционал

- Вращение планеты вокруг своей оси  
- Анимация столкновения метеора с планетой  
- Реалистичное разлетание осколков (каждый осколок имеет свой размер и массу, что влияет на скорость)
- Клавиша `f` — выстрел метеоритом  
- Клавиша `g` — откат к начальному состоянию  
- Движение Луны по орбите  
- Поддержка текстур на объектах  
- Различные типы освещения (точечный и направленный)  
- Использование нескольких материалов (блеск, цвет, отражения)  
- Реализация теней  

## 📝 Описание

Проект создан в рамках лабораторной работы по компьютерной графике. Основная цель — реализация визуального и физически достоверного процесса разрушения планеты при столкновении с метеором. Присутствует управление сценой в реальном времени, а также взаимодействие между объектами.

## 🤖 Стек

- C++
- OpenGL (GLUT, GLU)
- stb_image (для загрузки текстур)

## 📸 Скрины
<img width="300" alt="image" src="https://github.com/user-attachments/assets/c39d6b25-9004-467e-808c-5d0ead3b175c" />
<img width="300" alt="image" src="https://github.com/user-attachments/assets/7cf0be27-f72c-4783-b7e0-ded516b78809" />
<img width="300" alt="image" src="https://github.com/user-attachments/assets/9c5d11a7-d2c1-4230-ab57-bc7fb57a848d" />
<img width="300" alt="image" src="https://github.com/user-attachments/assets/ba15abc3-d12a-463f-9a52-12b032b4745e" />

## ⚡ Инструкция по запуску

### На Windows
1. Клонируй репозиторий: git clone https://github.com/Kurillccc/PlanetExplosionSimulator
2. Добавь библиотеки в проект:
    - В свойствах проекта добавь папки, содержащие заголовочные файлы библиотек stb_image и glut, чтобы компилятор мог их найти:
      - Перейди в Properties проекта.
      - В разделе VC++ Directories добавь путь к папке с заголовочными файлами glut.h и stb_imahe.h (уже лежат в проекте /OpenGL/OpenGL) в Include Directories.
3. Добавь библиотеки для линковки:
    - В разделе Linker -> Input добавь glut32.lib в Additional Dependencies
