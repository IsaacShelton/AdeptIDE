
#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

bool openFileDialog(GLFWwindow* window, std::string& output);
bool openMultipleFileDialog(GLFWwindow* window, std::vector<std::string>& output);
bool saveFileDialog(GLFWwindow* window, std::string& output);

#endif // FILEDIALOG_H