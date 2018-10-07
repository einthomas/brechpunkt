#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

int main() {
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwSwapInterval(1);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    window = glfwCreateWindow(
        //mode->width, mode->height,
		1280, 720, "Brechpunkt", nullptr, nullptr
    );
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        // TODO

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    return 0;
}
