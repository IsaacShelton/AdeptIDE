
#ifndef ADEPTIDE_H_INCLUDED
#define ADEPTIDE_H_INCLUDED

#include <vector>

#include "INTERFACE/TextEditor.h"
#include "INTERFACE/MenuBar.h"
#include "INTERFACE/Message.h"
#include "INTERFACE/Settings.h"
#include "PROCESS/FolderWatcher.h"

class AdeptIDE {
private:
    Font font;
    FolderWatcher rootWatcher;

    void handleInput();
    void update();
    void renderEditorFilenames();
    void loadSettings();

public:
    std::string root;
    GLFWwindow *window;
    int width, height;
    Matrix4f projectionMatrix;
    Matrix4f transformationMatrix;
    Settings settings;

    MenuBar menubar;
    std::vector<GenericEditor*> editors;
    size_t currentEditorIndex;

    bool mouseReleased;
    float mouseDownX, mouseDownY, mouseDownNetXOffset, mouseDownNetYOffset;
    size_t mouseDownCaretPosition;

    std::string assetsFolder;
    Model *emblemModel, *plainTextModel, *adeptModel, *javaModel, *htmlModel, *paintingModel;
    Texture *fontTexture, *emblemTexture, *plainTextTexture, *adeptTexture, *javaTexture, *htmlTexture, *paintingTexture;
    Shader *shader, *fontShader, *solidShader;

    Message *message;

    AdeptIDE();
    ~AdeptIDE();
    int main(int argc, const char **argv);

    void openFile();
    void openEditor(const std::string& filename);
    void newFile(FileType fileType);
    GenericEditor* getCurrentEditor();
    TextEditor *getCurrentEditorAsTextEditor();
    ImageEditor *getCurrentEditorAsImageEditor();
    TextEditor* addTextEditor();
    ImageEditor* addImageEditor();
    void removeEditor(size_t index);
    void removeCurrentEditor();
    void moveToPreviousEditorTab();
    void moveToNextEditorTab();
    void updateTitle();
    void setCurrentEditor(size_t index);

    void type(const std::string& text);
    void type(char character);
    void typeBlock();
    void typeExpression();
    void typeArrayAccess();
    void typeString();
    void typeCString();
    void backspace();
    void del();
    void smartBackspace();
    void smartDel();
    void backspaceLine();
    void delLine();
    void startSelection();
    void endSelection();
    void destroySelection();
    void deleteSelected();
    void copySelected();
    void cutSelected();
    void paste();
    void tab();
    void nextLine();
    void nextPrecedingLine();
    void finishSuggestion();

    void moveCaret(double xpos, double ypos);
    void moveCaretLeft();
    void moveCaretRight();
    void moveCaretUp();
    void moveCaretDown();
    void moveCaretBeginningOfLine();
    void moveCaretEndOfLine();
    void moveCaretBeginningOfWord();
    void moveCaretEndOfWord();
    void moveCaretBeginningOfSubWord();
    void moveCaretEndOfSubWord();
    void moveCaretOutside();
    void scrollDown(int lineCount);
    void scrollUp(int lineCount);
    void pageUp();
    void pageDown();
    void selectAll();
    void selectLine();

    void duplicateCaretUp();
    void duplicateCaretDown();

    void saveFile();

    void createMessage(const std::string& message, double seconds);
};

void scroll_callback(GLFWwindow *window, double xOffset, double yOffset);

void character_callback(GLFWwindow *window, unsigned int codepoint);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void handle_left_click(AdeptIDE *adeptide, double xpos, double ypos);

void drop_callback(GLFWwindow *window, int count, const char **paths);

void file_menu(void *data);
void view_menu(void *data);
void selection_menu(void *data);
void build_menu(void *data);

void new_adept_file(void *data);
void new_plain_text_file(void *data);
void new_java_file(void *data);
void new_painting_file(void *data);

void open_playground_menu(void *data);
void open_file_menu(void *data);
void save_file_menu(void *data);
void close_file_menu(void *data);
void settings_menu(void *data);
void quit_menu(void *data);

void select_all_menu(void *data);

void language_plain(void *data);
void language_adept(void *data);
void language_java(void *data);
void language_html(void *data);

void theme_visual_studio(void *data);
void theme_fruit_smoothie(void *data);
void theme_tropical_ocean(void *data);
void theme_island_campfire(void *data);
void theme_one_dark(void *data);

void maximize(void *data);

void build_adept_project(void *data);
void build_and_run_adept_project(void *data);

double distance(double x1, double y1, double x2, double y2);

#endif // ADEPTIDE_H_INCLUDED
