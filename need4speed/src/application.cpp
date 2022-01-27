#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glez.h"
#include "exp_scene.h"

#include <glm/vec2.hpp>

/* Core program */

bool initGLFW();
bool initWindow();
bool initGLEW();
bool initGL();
void scroll_callback(GLFWwindow* _window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* _window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* _window, double xpos, double ypos);
void key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods);

static float screenWidth = 1200.f;
static float screenHeight = 900.f;
static GLFWwindow* window;
static exp_scene* scene;
static bool is_dragging = false;
static glm::vec2 ref_pos;

int main(int argc, char** argv) 
{
    if (!initGLFW()) return -1;
    if (!initWindow()) return -1;
    if (!initGLEW()) return -1;
    if (!initGL()) return -1;

	glez::init();
	scene = new exp_scene();
    scene->get_camera()->set_proj_props(screenWidth, screenHeight);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene->display();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
	delete scene;
	return 0;
}

/* Basic initializations */

bool initGLFW()
{
    if (!glfwInit())
        return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    return true;
}

bool initWindow()
{
    window = glfwCreateWindow((int)screenWidth, (int)screenHeight, "need4speed", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);
    return true;
}

bool initGLEW()
{
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        return false;
    }
    return true;
}

bool initGL()
{
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    return true;
}

/* Callbacks */

void scroll_callback(GLFWwindow* _window, double xoffset, double yoffset)
{
    float lambda = 1.f - (float)yoffset * 0.1f;
    scene->get_camera()->scale_view(lambda);
}

void mouse_button_callback(GLFWwindow* _window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            is_dragging = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            ref_pos.x = 2 * ((float)xpos / screenWidth) - 1;
            ref_pos.y = 1 - 2 * ((float)ypos / screenHeight);
        }
        else {
            is_dragging = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* _window, double xpos, double ypos)
{
    if (is_dragging) {
        glm::vec2 cur_pos = glm::vec2(2 * ((float)xpos / screenWidth) - 1,
            1 - 2 * ((float)ypos / screenHeight));
        glm::vec2 delta = cur_pos - ref_pos;
        GLfloat theta_x = delta.x * 3.14f * 0.5f;
        GLfloat theta_y = delta.y * 3.14f * 0.5f;
        scene->get_camera()->rotate_view(-theta_x, -theta_y);
        ref_pos = cur_pos;
    }
}

void key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods)
{
    // TODO
}
