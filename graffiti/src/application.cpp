#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <iostream>

#include "glez.h"
#include "graffiti_scene.h"

#include <glm/vec2.hpp>

/* Core program */

enum class Activity 
{
    Select, Fill, Paint, Extrude, Cut, Displace
};

bool initGLFW();
bool initWindow();
bool initGLEW();
bool initGL();
void scroll_callback(GLFWwindow* _window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* _window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* _window, double xpos, double ypos);
void key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods);
void set_activity(Activity _activity);

static float screenWidth = 900.f;
static float screenHeight = 600.f;
static GLFWwindow* window;
static graffiti_scene* scene;

static Activity activity = Activity::Fill;
static glm::vec2 ref_pos;
static bool is_dragging = false;
static bool is_rotating = false;

int main(int argc, char** argv)
{
    if (!initGLFW()) return -1;
    if (!initWindow()) return -1;
    if (!initGLEW()) return -1;
    if (!initGL()) return -1;

    glez::init();
    scene = new graffiti_scene();
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
    window = glfwCreateWindow((int)screenWidth, (int)screenHeight, "graffiti", NULL, NULL);
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
            if (mods == GLFW_MOD_ALT) {
                is_rotating = true;
            }
            else {
                switch (activity) {
                case Activity::Fill:
                    scene->fill(ref_pos);
                    break;
                case Activity::Paint:
                    scene->spraypaint(ref_pos);
                    break;
                default:
                    break;
                }
            }
        }
        else {
            is_dragging = false;
            is_rotating = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* _window, double xpos, double ypos)
{
    if (is_dragging) {
        glm::vec2 cur_pos = glm::vec2(2 * ((float)xpos / screenWidth) - 1,
            1 - 2 * ((float)ypos / screenHeight));
        glm::vec2 delta = cur_pos - ref_pos;
        if (is_rotating) {
            GLfloat theta_x = delta.x * 3.14f * 0.5f;
            GLfloat theta_y = delta.y * 3.14f * 0.5f;
            scene->get_camera()->rotate_view(-theta_x, -theta_y);
        }
        else {
            switch (activity) {
            case Activity::Paint:
                scene->spraypaint(cur_pos);
                break;
            default:
                break;
            }
        }
        ref_pos = cur_pos;
    }
}

void key_callback(GLFWwindow* _window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_H && action == GLFW_RELEASE) {
        // help
        std::cout << "H: help" << std::endl;
        std::cout << "I: object info" << std::endl;
        std::cout << "S: select mode" << std::endl;
        std::cout << "F: fill mode" << std::endl;
        std::cout << "P: spraypaint mode" << std::endl;
        std::cout << "E: extrude mode" << std::endl;
        std::cout << "C: cut mode" << std::endl;
        std::cout << "D: displace mode" << std::endl;
    }
    else if (key == GLFW_KEY_I && action == GLFW_RELEASE) {
        // object info
        scene->log_object_info();
    }
    else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        // select mode
        set_activity(Activity::Select);
    }
    else if (key == GLFW_KEY_F && action == GLFW_RELEASE) {
        // fill mode
        set_activity(Activity::Fill);
    }
    else if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
        // spraypaint mode
        set_activity(Activity::Paint);
    }
    else if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
        // extrude mode
        set_activity(Activity::Extrude);
    }
    else if (key == GLFW_KEY_C && action == GLFW_RELEASE) {
        // cut mode
        set_activity(Activity::Cut);
    }
    else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
        // displace mode
        set_activity(Activity::Displace);
    }
}

void set_activity(Activity _activity)
{
    activity = _activity;
}
