#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
double mouseX, mouseY;

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 ourColor;
uniform vec2 iResolution;
uniform float iTime;
uniform vec4 iMouse;
uniform sampler2D iChannel1;

vec3 palette(float t){ // магия сочно политрового градиента

vec3 a = vec3(0.5,0.5,0.5);
vec3 b = vec3(0.5,0.5,0.5);
vec3 c = vec3(1.0,1.0,1.0);
vec3 d = vec3(0.263,0.416,0.557);

return a+b*cos( 6.28318*(c*t+d) );
}



void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
//чтобы программа не зависела от размера холста
//нужен переводчик из формата окна в формат в формат диапозона [0.1]

//iResolution.xy - вектор текущего разрешения
//iResolution = vec3(x,y,z) или (width,height,depth), depth - только для двухмерностей
vec2 uv = fragCoord / iResolution.xy;

//по умолчанию центр расположен имеет координаты 1/2 от размеров холста
//для удобсва, следует сделать их (0,0)

uv -= 0.5; //сместили центр в середину (теперье го координаты (0,0) )
uv *= 2.0; //теперь края идут в диапозоне от (-1,1), а не (-0.5,0.5)

// упращённая запись: vec2 uv = fragCoord / iResolution.xy * 2.0 - 1.0;


// градиент(чёрный - красный): fragColor = vec4(uv.x,0.0,0.0,1.0);

uv.x *= iResolution.x/iResolution.y; // устраняет проблему пропорцинальности размера круга относительно холста

vec2 uv0 = uv; // координата глобального нуля

vec3 finalColor = vec3(0.0);

for (float i = 0.0; i < 3.5; i++){

    uv = fract(uv * 1.5) - 0.5; // переходим в режим фракталов

    float d = length(uv) * exp(-length(uv0)); // определяет длинну от текущей точки до центра

    float freqTime = 0.7; // "частота" времени: freqTime<1.0 - быстрее; freqTime>1.0 - медленнее
    vec3 coolColor = palette(length(uv0) +i*freqTime + iTime/freqTime);

    float thick = 12.0, freq = 2.0;

    d = sin(d*thick + iTime)/freq; // магия синуса: d = sin (d* {частота кругов})/{толщина окружности}
    d = abs(d); // обсалютная величина
    //d -= 0.5; // магия с размером


    //d = step (0.1,d); //отступ с шагом | жёсткая

    d = 0.001/d; //эффект свячения за счёт обратной функции

    d = smoothstep(0.0,0.1,d); // мягкий шаг (порог1, порог2,x точки)

   // d = pow (0.01/ d, 1.2);
    finalColor += coolColor*d;
}



//    float red = d,green = d,  blue = d; // параметры цвета
    //vec3 coolColor = vec3(1.0,2.0,3.0); // используемый цвет
//    coolColor *= d; //зависимость от растояния
    //fragColor = vec4(red,green,blue,1.0);


fragColor = vec4(finalColor,1.0);
// fragCoord vec2(x,y) - координата текущего пикселя
// fragColor vec4(red,green,blue,Alpha) - канал для цвета и Альфа канал - прозрачность пикселя

}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    mainImage(FragColor, fragCoord);
}
)";


float coord = 0.7f;
//GLfloat vertices[] = {
//    coord, -coord, 0.0f,
//    -coord, -coord, 0.0f,
//    -coord,  coord, 0.0f,
//    coord,  coord, 0.0f,
//coord, -coord, 0.0f
//};

GLfloat vertices[] = {
    coord, 0.0f, 0.0f,
     0.0f, coord, 0.0f,
    -coord,  0.0f, 0.0f,
    0.0f, -coord, 0.0f
};


int al = sizeof(vertices)/sizeof(vertices[0]); //вычисление числа вершин

unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource)
{
    auto compileShader = [](GLenum type, const char* source) {
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        return shader;
    };

    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}


int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef APPLE
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Shaders", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height){ glViewport(0, 0, width, height); });
    glfwSetCursorPosCallback(window, [](GLFWwindow*, double xpos, double ypos){ mouseX = xpos; mouseY = ypos; });

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "iChannel1"), 0);

        double timeValue = glfwGetTime();
        glUniform2f(glGetUniformLocation(shaderProgram, "iResolution"), SCR_WIDTH, SCR_HEIGHT);
        glUniform1f(glGetUniformLocation(shaderProgram, "iTime"), static_cast<float>(timeValue));
        glUniform2f(glGetUniformLocation(shaderProgram, "iMouse"), static_cast<float>(mouseX), static_cast<float>(SCR_HEIGHT - mouseY));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, al);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
