#include <windows.h>
#include <glut.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLuint loadTexture(const char* filename);

GLuint earthTexture;
GLuint moonTexture;
GLuint skyTexture;
GLuint meteorTexture;


float angle = 0.0f;
bool exploding = false;
bool restoring = false;
bool planetExists = true;
float explosionPower = 0.8f;
float earthRotationAngle = 0.0f;

float moonSavedX = 0.0f, moonSavedY = 0.0f, moonSavedZ = 0.0f;
float moonMoveSpeedX = 0.0f, moonMoveSpeedY = 0.0f, moonMoveSpeedZ = 0.0f;
bool moonMoved = false;

float lightDistance = 5.0f;
float moonOrbitSpeed = 0.9f;
std::vector<struct Particle> particles;

float skyRotationAngle = 0.2f; // Угол вращения для небесной сферы


struct Particle {
    float x, y, z;      // Позиция обломка
    float vx, vy, vz;   // Скорость обломка
    float mass;         // Масса (влияет на движение)
    float size;         // Размер обломка
};

float projectileX = -8, projectileY = 5, projectileZ = 5;
bool projectileMoving = false;
float projectileVX, projectileVY, projectileVZ;

bool isMoonInShadow(float lightX, float lightY, float lightZ, float moonX, float moonY, float moonZ, float earthRotationAngle) {
    // Центр планеты (Земли)
    float planetX = 0, planetY = 0, planetZ = 0;

    // Позиция Земли с учётом её вращения
    float earthRotationOffsetX = cos(earthRotationAngle);
    float earthRotationOffsetZ = sin(earthRotationAngle);

    // Вектор от источника света (Солнца) к Земле
    float toPlanetX = planetX - lightX;
    float toPlanetY = planetY - lightY;
    float toPlanetZ = planetZ - lightZ;

    // Вектор от источника света (Солнца) к Луне
    float toMoonX = moonX - lightX;
    float toMoonY = moonY - lightY;
    float toMoonZ = moonZ - lightZ;

    // Длина векторов
    float len1 = sqrt(toPlanetX * toPlanetX + toPlanetY * toPlanetY + toPlanetZ * toPlanetZ);
    float len2 = sqrt(toMoonX * toMoonX + toMoonY * toMoonY + toMoonZ * toMoonZ);

    // Нормализация векторов
    toPlanetX /= len1; toPlanetY /= len1; toPlanetZ /= len1;
    toMoonX /= len2; toMoonY /= len2; toMoonZ /= len2;

    // Проверяем, если Луна находится за Землёй относительно Солнца
    float dotProduct = toPlanetX * toMoonX + toPlanetY * toMoonY + toPlanetZ * toMoonZ;

    // Если угол между направлениями небольшой и Луна находится далеко за планетой (на противоположной стороне) - это тень
    return dotProduct > 0.5f && len2 > len1;
}

void initParticles() {
    particles.clear();
    for (int i = 0; i < 3300; i++) {
        Particle p;
        p.x = (rand() % 200 - 100) / 50.0f;  // Случайная позиция относительно центра
        p.y = (rand() % 200 - 100) / 50.0f;
        p.z = (rand() % 200 - 100) / 50.0f;
        p.vx = ((rand() % 200) - 100) / 100.0f;
        p.vy = ((rand() % 200) - 100) / 100.0f;
        p.vz = ((rand() % 200) - 100) / 100.0f;
        p.mass = 0.1f + (rand() % 200) / 10.0f;  // Масса между 0.5 и 1.0
        p.size = 0.1f + (rand() % 300) / 10.0f;
        particles.push_back(p);
    }
}

void drawFragment(float size) {
    // Рисуем многоугольник (или кубик) как кусок планеты
    glBegin(GL_TRIANGLES);
    glVertex3f(0, 0, 0);
    glVertex3f(size, size, 0);
    glVertex3f(0, size, size);
    glEnd();
}

void drawSky() {
    glPushMatrix();

    // Вращаем небесную сферу, чтобы создать эффект вращения звезд
    glRotatef(angle * 6.0f, 0.0f, 1.0f, 0.0f);

    // Переворачиваем нормали (чтобы текстура была внутри сферы)
    glFrontFace(GL_CW);

    glBindTexture(GL_TEXTURE_2D, skyTexture);
    GLUquadric* quadric = gluNewQuadric();
    gluQuadricTexture(quadric, GL_TRUE);

    // Рисуем сферу, которая окружает сцену
    gluSphere(quadric, 60.0, 50, 50);  // Радиус сферы большой, чтобы она покрывала всю сцену

    gluDeleteQuadric(quadric);

    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(-5, 5, 10, 0, 0, 0, 0, 1, 0);

    glDisable(GL_LIGHTING);
    glEnable(GL_LIGHTING);


    GLfloat light_pos[] = {
        lightDistance * cos(angle),
        5.0f,
        lightDistance * sin(angle),
        1.0f
    };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    float moonX, moonZ;
    if (!moonMoved) {
        moonX = 2 * cos(angle * moonOrbitSpeed);
        moonZ = -2 * sin(angle * moonOrbitSpeed);
    }
    else {
        moonX = moonSavedX;
        moonZ = moonSavedZ;
    }


    if (planetExists && !exploding) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, earthTexture);
        GLUquadric* earth = gluNewQuadric();
        gluQuadricTexture(earth, GL_TRUE);
        gluSphere(earth, 1.0, 50, 50);
        gluDeleteQuadric(earth);
        glDisable(GL_TEXTURE_2D);

    }
    else if (exploding) {
        glColor3f(1, 0.5, 0);

        // Устанавливаем начальную скорость частиц, чтобы они сразу начинали двигаться с достаточной скоростью
        float initialSpeed = 0.3f; // Задать начальную скорость

        // Заменяем отображение частиц на куски, которые летят от центра планеты
        for (auto& p : particles) {
            // Вычисляем направление от центра планеты
            float dirX = p.x;
            float dirY = p.y;
            float dirZ = p.z;
            float length = sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
            dirX /= length;
            dirY /= length;
            dirZ /= length;

            // Применяем начальную скорость
            p.vx = dirX * initialSpeed; // Начальная скорость по X
            p.vy = dirY * initialSpeed; // Начальная скорость по Y
            p.vz = dirZ * initialSpeed; // Начальная скорость по Z

            // Обновляем позиции частиц
            p.x += p.vx;
            p.y += p.vy;
            p.z += p.vz;

            glPushMatrix();
            glTranslatef(p.x, p.y, p.z);
            glutSolidCube(0.1); // Отображаем частицы как кубики
            glPopMatrix();
        }
    }


    glDisable(GL_LIGHTING);
    
    glPushMatrix();
    glTranslatef(moonX, 0, moonZ);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, moonTexture);

    // Проверка на тень Луны
    if (isMoonInShadow(light_pos[0], light_pos[1], light_pos[2], moonX, 0, moonZ, earthRotationAngle)) {
        glColor3f(0.3, 0.3, 0.3); // Затемнённая Луна
    }
    else {
        glColor3f(0.7, 0.7, 0.7); // Освещённая Луна
    }

    GLUquadric* moon = gluNewQuadric();
    gluQuadricTexture(moon, GL_TRUE);
    gluSphere(moon, 0.27, 50, 50);  // Нарисовать Луну с текстурой
    gluDeleteQuadric(moon);

    glDisable(GL_TEXTURE_2D); // Отключаем текстуры для остальных объектов
    glPopMatrix();
    
    // Рисуем небесную сферу (звезды) с нормальной яркостью
    glDisable(GL_LIGHTING);  // Отключаем освещение только для небесной сферы
    glColor3f(1.0, 1.0, 1.0);  // Задаем нормальный цвет для звезд
    glEnable(GL_TEXTURE_2D); drawSky(); glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);  // Включаем освещение снова


    if (projectileMoving) {
        glDisable(GL_LIGHTING);
        glColor3f(1, 1, 1); // Снаряд будет с нормальным освещением, т.к. применяем текстуру
        glPushMatrix();
        glTranslatef(projectileX, projectileY, projectileZ);
        
        
        glEnable(GL_TEXTURE_2D); // Включаем текстуры
        glBindTexture(GL_TEXTURE_2D, meteorTexture); // Применяем текстуру снаряда
        GLUquadric* meteor = gluNewQuadric();
        gluQuadricTexture(meteor, GL_TRUE);
        gluSphere(meteor, 0.2, 20, 20);
        gluDeleteQuadric(meteor); // Отображаем снаряд с текстурой
        glDisable(GL_TEXTURE_2D); // Отключаем текстуру

        glPopMatrix();
        glEnable(GL_LIGHTING); // Включаем освещение снова
    }



    glutSwapBuffers();  

}

void timer(int value) {
    angle += 0.01f;
    skyRotationAngle += 0.03f; // Увеличиваем угол вращения звёзд

    if (projectileMoving) {
        projectileX += projectileVX;
        projectileY += projectileVY;
        projectileZ += projectileVZ;
        

        float dx = projectileX;
        float dy = projectileY;
        float dz = projectileZ;
        float distance = sqrt(dx * dx + dy * dy + dz * dz);

        if (distance <= 1.0f && planetExists) {
            exploding = true;
            restoring = false;
            explosionPower = 0;
            initParticles();
            planetExists = false;
            projectileMoving = false;

            float orbitX = 2 * cos(angle * moonOrbitSpeed);
            float orbitZ = -2 * sin(angle * moonOrbitSpeed);
            float dirX = orbitX;
            float dirY = 0;
            float dirZ = orbitZ;
            float length = sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
            dirX /= length;
            dirY /= length;
            dirZ /= length;

            moonSavedX = orbitX;
            moonSavedY = 0;
            moonSavedZ = orbitZ;
            moonMoveSpeedX = dirX * 0.5f;
            moonMoveSpeedY = dirY * 0.5f;
            moonMoveSpeedZ = dirZ * 0.5f;
            moonMoved = true;
        }
    }

    if (exploding) {
        explosionPower += 0.01f;
        for (auto& p : particles) {
            p.x += p.vx * explosionPower;
            p.y += p.vy * explosionPower;
            p.z += p.vz * explosionPower;
        }
    }

    if (restoring) {
        bool done = true;
        for (auto& p : particles) {
            p.x *= 0.7f;
            p.y *= 0.7f;
            p.z *= 0.7f;
            if (fabs(p.x) > 0.6 || fabs(p.y) > 0.6 || fabs(p.z) > 0.6)
                done = false;
        }
        if (done) {
            restoring = false;
            exploding = false;
            planetExists = true;
            projectileX = -8;
            projectileY = 5;
            projectileZ = 5;
        }
    }

    if (moonMoved) {
        moonSavedX += moonMoveSpeedX;
        moonSavedY += moonMoveSpeedY;
        moonSavedZ += moonMoveSpeedZ;

        moonMoveSpeedX *= 0.98f;
        moonMoveSpeedY *= 0.98f;
        moonMoveSpeedZ *= 0.98f;
    }

    glutPostRedisplay();
    glutTimerFunc(1000 / 120, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) exit(0);

    if (key == 'f' && !projectileMoving && planetExists) {
        projectileMoving = true;
        float dx = 0 - projectileX;
        float dy = 0 - projectileY;
        float dz = 0 - projectileZ;
        float length = sqrt(dx * dx + dy * dy + dz * dz);
        projectileVX = (dx / length) * 0.1f;
        projectileVY = (dy / length) * 0.1f;
        projectileVZ = (dz / length) * 0.1f;
    }

    if (key == 'g' && !planetExists) {
        restoring = true;
        explosionPower = 0;
        projectileX = -8;
        projectileY = 5;
        projectileZ = 5;
        projectileMoving = false;
        projectileVX = 0;
        projectileVY = 0;
        projectileVZ = 0;
        planetExists = true;
        exploding = false;
        particles.clear();
        moonMoved = false;
        moonSavedX = moonSavedY = moonSavedZ = 0;
        moonMoveSpeedX = moonMoveSpeedY = moonMoveSpeedZ = 0;
    }
}

GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);

    if (!image) {
        printf("Failed to load texture: %s\n", filename);
        printf("stbi_load error: %s\n", stbi_failure_reason());  // Выводим причину ошибки
        exit(1);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    // Загружаем текстуру и создаем мип-маппинг
    gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, image);

    // Настройки текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // Округление по горизонтали
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  // Округление по вертикали

    // Включение мип-маппинга для масштабирования текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  // Использование мип-маппинга для уменьшения
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // Линейная фильтрация для увеличения

    stbi_image_free(image);
    return textureID;
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Установить чёрный фон
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Очистить экран
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 800.0 / 600.0, 1, 100);
    glMatrixMode(GL_MODELVIEW);

    

    glEnable(GL_TEXTURE_2D); // Включаем текстуры
    earthTexture = loadTexture("earth.jpg");
    moonTexture = loadTexture("moon.jpg");
    skyTexture = loadTexture("stars_texture.jpg");
    meteorTexture = loadTexture("meteor.jpg");
}

int main(int argc, char** argv) {
    srand(time(0));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glEnable(GL_MULTISAMPLE);
    glutInitWindowSize(1000, 750);
    glutCreateWindow("Planet Explosion Simulation");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}
