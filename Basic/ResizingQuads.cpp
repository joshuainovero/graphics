#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <utility>
#include <fstream>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <vector>
#include <tuple>

static GLfloat g_buffer_pos1 [] = {
    -0.5f, -0.5f,
     0.5f, -0.5f,
     0.5f,  0.5f,
    -0.5f,  0.5f
};

static constexpr unsigned int q_indices [] = {
    0, 1, 2,
    2, 3, 0
};

static std::pair<std::string, std::string> ParseShader(const std::string& vsFile, const std::string& fsFile){
    std::stringstream vsSS, fsSS;
    std::string line;

    std::ifstream vsStream(vsFile);
        while(std::getline(vsStream, line))
            vsSS << line << "\n";
    vsStream.close();

    std::ifstream fsStream(fsFile);
        while(std::getline(fsStream, line))
            fsSS << line << "\n";
    fsStream.close();

    return std::make_pair(vsSS.str(), fsSS.str());
}

static GLuint CompileShader(int type, const std::string& source){
    GLuint shaderID = glCreateShader(type);
    const char* source_c = source.c_str();
    glShaderSource(shaderID, 1, &source_c, nullptr);
    glCompileShader(shaderID);

    GLint s_Result, s_Length;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &s_Result);
    if (s_Result == GL_FALSE){
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &s_Length);
        char* s_Error = (char*)alloca(s_Length * sizeof(char));
        glGetShaderInfoLog(shaderID, s_Length, &s_Length, s_Error);
        std::cout << "Failed to compile " << ((type == GL_VERTEX_SHADER) ? "vertex" : "fragment") << "shader" << std::endl;
        std::cout << s_Error << std::endl;
        glDeleteShader(shaderID);
    }

    return shaderID;
}

static GLuint LoadShaderSource(const std::string& vsSource, const std::string& fsSource){
    GLuint shaderProgram = glCreateProgram();

    GLuint vsID = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fsID = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(shaderProgram, vsID);
    glAttachShader(shaderProgram, fsID);
    glLinkProgram(shaderProgram);
    glValidateProgram(shaderProgram);

    glDeleteShader(vsID);
    glDeleteShader(fsID);

    return shaderProgram;
}

static void CreateQuads(GLuint& vao, GLuint& buffer, GLuint& ibo, const GLfloat g_buffer_data []){
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, g_buffer_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, q_indices, GL_STATIC_DRAW);

}

void ReduceQuadsSize(GLfloat reducer, GLfloat g_buffer_pos []){
    g_buffer_pos[0] += reducer;
    g_buffer_pos[1] += reducer;
    g_buffer_pos[2] -= reducer;
    g_buffer_pos[3] += reducer;
    g_buffer_pos[4] -= reducer;
    g_buffer_pos[5] -= reducer;
    g_buffer_pos[6] += reducer;
    g_buffer_pos[7] -= reducer;

}

static float r_color = 0.0f;
static float r_increment = 0.001f;
static GLint s_counter = 0;
static GLint s_index = 0;
static GLint s_incr = 1;

int main(){

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Window", NULL, NULL);

    glfwMakeContextCurrent(window);

    glewInit();

    std::vector<std::tuple<GLuint, GLuint, GLuint>> vaob;

    for (int i = 0; i < 30; ++i){
        GLuint vao, buffer, ibo;
        CreateQuads(vao, buffer, ibo, g_buffer_pos1);
        ReduceQuadsSize(0.01f, g_buffer_pos1);
        vaob.push_back(std::make_tuple(vao, buffer, ibo));
    }

    glBindVertexArray(std::get<0>(vaob[0]));
    glBindBuffer(GL_ARRAY_BUFFER, std::get<1>(vaob[0]));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, std::get<2>(vaob[0]));

    auto shaderSource = ParseShader("vert.shader", "frag.shader");
    GLuint shaderProgram = LoadShaderSource(shaderSource.first, shaderSource.second);
    glUseProgram(shaderProgram);

    GLint u_Location = glGetUniformLocation(shaderProgram, "u_Color");
    assert(u_Location != -1);
    glUniform4f(u_Location, 0.0f,  0.3f, 0.8f, 1.0f);


    do {
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform4f(u_Location, r_color, 0.3f, 0.8f, 1.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        if (r_color > 1.0f) r_increment = -0.001f;
        else if (r_color < 0.0f) r_increment = 0.001f;
        r_color += r_increment;

        if (s_counter >= 20){
            if (s_index > vaob.size() - 2)
                s_incr = -1;
            else if (s_index <= 0)
                s_incr = 1;

            glBindVertexArray(std::get<0>(vaob[s_index]));
            glBindBuffer(GL_ARRAY_BUFFER, std::get<1>(vaob[s_index]));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, std::get<2>(vaob[s_index]));
            s_index += s_incr;
            s_counter = 0;
        }

        s_counter++;
        glfwSwapBuffers(window);

        glfwPollEvents();
    }
    while (!glfwWindowShouldClose(window));

    glfwTerminate();
    return 0;
}