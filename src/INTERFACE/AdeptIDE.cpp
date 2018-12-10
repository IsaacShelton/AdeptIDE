
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include "UTIL/dialog.h"
#include "UTIL/strings.h"
#include "UTIL/lexical.h"
#include "UTIL/filename.h"
#include "INTERFACE/AdeptIDE.h"
#include "INTERFACE/quicktype.h"
#include "PROCESS/background.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

AdeptIDE::AdeptIDE(){
    this->fontTexture = NULL;

    this->window = NULL;
    this->mouseReleased = true;
    this->mouseDownX = 0.0f;
    this->mouseDownY = 0.0f;
    this->currentEditorIndex = 0;

    this->emblemModel       = NULL;
    this->plainTextModel    = NULL;
    this->adeptModel        = NULL;
    this->javaModel         = NULL;
    this->htmlModel         = NULL;
    this->fontTexture       = NULL;
    this->emblemTexture     = NULL;
    this->plainTextTexture  = NULL;
    this->adeptTexture      = NULL;
    this->javaTexture       = NULL;
    this->htmlTexture       = NULL;
    this->shader            = NULL;
    this->fontShader        = NULL;
    this->solidShader       = NULL;
}

AdeptIDE::~AdeptIDE(){
    delete this->emblemModel;
    delete this->plainTextModel;
    delete this->adeptModel;
    delete this->javaModel;
    delete this->htmlModel;
    delete this->fontTexture;
    delete this->emblemTexture;
    delete this->plainTextTexture;
    delete this->adeptTexture;
    delete this->javaTexture;
    delete this->htmlTexture;
    delete this->shader;
    delete this->fontShader;
    delete this->solidShader;

    for(Editor* editor : this->editors){
        editor->free();
        delete editor;
    }
}

int AdeptIDE::main(int argc, const char **argv){
    if(!glfwInit()){
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }

    // Determine root directory of executable
    char location[512];

    #ifdef _WIN32
    GetModuleFileNameA(NULL, location, 512);
    #else
    #error "compiler_invoke doesn't have GetModuleFileNameA for this platform"
    #endif

    char *absolute_filename = filename_absolute(location);
    char *absolute_path = filename_path(absolute_filename);
    this->root = absolute_path;
    this->assetsFolder = this->root + "assets\\";
    free(absolute_path);
    free(absolute_filename);

    // Load settings
    this->rootWatcher.target(this->root);
    this->loadSettings();

    const bool fullscreen = false;
    glfwWindowHint(GLFW_MAXIMIZED, this->settings.ide_default_maximized);

    if(fullscreen){
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
        this->width = vidmode->width;
        this->height = vidmode->height;
        this->window = glfwCreateWindow(this->width, this->height, "Adept IDE", monitor, NULL);
    } else {
        this->width = this->settings.ide_default_width;
        this->height = this->settings.ide_default_height;
        this->window = glfwCreateWindow(this->width, this->height, "AdeptIDE", NULL, NULL);
    }    


    glfwMakeContextCurrent(this->window);
    glfwSwapInterval(this->settings.ide_default_fps <= 0 ? 0 : 60 / this->settings.ide_default_fps);

    if(glewInit() != GLEW_OK){
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return 1;
    }

    if(!GLEW_VERSION_3_0){
        std::cerr << "OpenGL 3.0 requirement not met" << std::endl;
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, &scroll_callback);
    glfwSetCharCallback(window, &character_callback);
    glfwSetKeyCallback(window, &key_callback);
    glfwSetMouseButtonCallback(window, &mouse_button_callback);
    glfwSetDropCallback(window, &drop_callback);

    // Load assets
    this->font.load(this->assetsFolder + "inconsolatago.fnt");
    this->fontTexture       = new Texture(this->assetsFolder + "inconsolatago.png", TextureLoadOptions::ALPHA_BLEND);
    this->emblemTexture     = new Texture(this->assetsFolder + "emblem.png", TextureLoadOptions::ALPHA);
    this->plainTextTexture  = new Texture(this->assetsFolder + "plaintext.png", TextureLoadOptions::ALPHA);
    this->adeptTexture      = new Texture(this->assetsFolder + "adept.png", TextureLoadOptions::ALPHA);
    this->javaTexture       = new Texture(this->assetsFolder + "java.png", TextureLoadOptions::ALPHA);
    this->htmlTexture       = new Texture(this->assetsFolder + "html.png", TextureLoadOptions::ALPHA);
    this->emblemModel       = makeSquareModel(this->emblemTexture, 256);
    this->plainTextModel    = makeSquareModel(this->plainTextTexture, 16);
    this->adeptModel        = makeSquareModel(this->adeptTexture, 16);
    this->javaModel         = makeSquareModel(this->javaTexture, 16);
    this->htmlModel         = makeSquareModel(this->htmlTexture, 16);
    this->shader            = new Shader(this->assetsFolder + "vertex.glsl",       this->assetsFolder + "fragment.glsl",       {"position", "uvs"});
    this->fontShader        = new Shader(this->assetsFolder + "font_vertex.glsl",  this->assetsFolder + "font_fragment.glsl",  {"position", "uvs"});
    this->solidShader       = new Shader(this->assetsFolder + "solid_vertex.glsl", this->assetsFolder + "solid_fragment.glsl", {"position"});

    this->menubar.load(&this->settings, &this->font, this->fontTexture, &this->editors);

    this->menubar.addMenu("FILE",      &file_menu,      NULL);
    this->menubar.addMenu("EDIT",      NULL,            NULL);
    this->menubar.addMenu("VIEW",      &view_menu,      NULL);
    this->menubar.addMenu("SELECTION", &selection_menu, NULL);
    this->menubar.addMenu("BUILD",     &build_menu,     NULL);
    this->menubar.addMenu("EXECUTE",   NULL,            NULL);
    this->menubar.addMenu("HELP",      NULL,            NULL);

    this->menubar.menus[0]->dropdownMenu = new DropdownMenu(&this->font, 4.0f, 20.0f, 32);
    this->menubar.menus[0]->data = this->menubar.menus[0]->dropdownMenu;
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("New File                  Ctrl+N", this->menubar.font, &new_file_menu, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Open File                 Ctrl+O", this->menubar.font, &open_file_menu, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Open Playground     Ctrl+Shift+N", this->menubar.font, &open_playground_menu, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Save File                 Ctrl+S", this->menubar.font, &save_file_menu, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Save File As        Ctrl+Shift+S", this->menubar.font, NULL, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Save all                  Ctrl+T", this->menubar.font, NULL, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Close File                Ctrl+W", this->menubar.font, &close_file_menu, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Settings              Ctrl+Comma", this->menubar.font, &settings_menu, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Quit                      Alt+F4", this->menubar.font, &quit_menu, this->window));

    float selectionDropDownX;

    selectionDropDownX = 8.0f + this->menubar.menus[0]->textWidth + this->menubar.menus[1]->textWidth + 16.0f * 2;
    this->menubar.menus[2]->dropdownMenu = new DropdownMenu(&this->font, selectionDropDownX, 20.0f, 28);
    this->menubar.menus[2]->data = this->menubar.menus[2]->dropdownMenu;

    // Change Language Menu
    DropdownMenu *changeLanguageDropdown = new DropdownMenu(&this->font, selectionDropDownX + 256.0f + 16.0f, 20.0f, 32);
    Menu *changeLanguageMenu = new Menu("Change Language             >", this->menubar.font, NULL, changeLanguageDropdown);
    changeLanguageMenu->dropdownMenu = changeLanguageDropdown;
    this->menubar.menus[2]->dropdownMenu->menus.push_back(changeLanguageMenu);
    changeLanguageDropdown->menus.push_back(new Menu("Plain Text                      ", this->menubar.font, language_plain, this));
    changeLanguageDropdown->menus.push_back(new Menu("Adept                     Ctrl+1", this->menubar.font, language_adept, this));
    changeLanguageDropdown->menus.push_back(new Menu("Java                      Ctrl+2", this->menubar.font, language_java, this));
    changeLanguageDropdown->menus.push_back(new Menu("HTML                      Ctrl+3", this->menubar.font, language_html, this));
    
    // Change Theme Menu
    DropdownMenu *changeThemeDropdown = new DropdownMenu(&this->font, selectionDropDownX + 256.0f + 16.0f, 20.0f, 32);
    Menu *changeThemeMenu = new Menu("Change Theme                >", this->menubar.font, NULL, changeThemeDropdown);
    changeThemeMenu->dropdownMenu = changeThemeDropdown;
    this->menubar.menus[2]->dropdownMenu->menus.push_back(changeThemeMenu);
    changeThemeDropdown->menus.push_back(new Menu("Visual Studio        Ctrl+Shift+1", this->menubar.font, theme_visual_studio, this));
    changeThemeDropdown->menus.push_back(new Menu("Tropical Ocean       Ctrl+Shift+2", this->menubar.font, theme_tropical_ocean, this));
    changeThemeDropdown->menus.push_back(new Menu("Island Campfire      Ctrl+Shift+3", this->menubar.font, theme_island_campfire, this));
    changeThemeDropdown->menus.push_back(new Menu("One Dark             Ctrl+Shift+4", this->menubar.font, theme_one_dark, this));
    changeThemeDropdown->menus.push_back(new Menu("Fruit Smoothie (Legacy)         ", this->menubar.font, theme_fruit_smoothie, this));

    selectionDropDownX = 8.0f + this->menubar.menus[0]->textWidth + this->menubar.menus[1]->textWidth + this->menubar.menus[2]->textWidth + 16.0f * 3;
    this->menubar.menus[3]->dropdownMenu = new DropdownMenu(&this->font, selectionDropDownX, 20.0f, 32);
    this->menubar.menus[3]->data = this->menubar.menus[3]->dropdownMenu;
    this->menubar.menus[3]->dropdownMenu->menus.push_back(new Menu("Select All                 Ctrl+A", this->menubar.font, select_all_menu, this));
    this->menubar.menus[3]->dropdownMenu->menus.push_back(new Menu("Expand Selection           Ctrl+E", this->menubar.font, NULL, this));

    selectionDropDownX = 8.0f + this->menubar.menus[0]->textWidth + this->menubar.menus[1]->textWidth + this->menubar.menus[2]->textWidth + this->menubar.menus[3]->textWidth + 16.0f * 4;
    this->menubar.menus[4]->dropdownMenu = new DropdownMenu(&this->font, selectionDropDownX, 20.0f, 32);
    this->menubar.menus[4]->data = this->menubar.menus[4]->dropdownMenu;
    this->menubar.menus[4]->dropdownMenu->menus.push_back(new Menu("Build                     Ctrl+B", this->menubar.font, build_adept_project, this));
    this->menubar.menus[4]->dropdownMenu->menus.push_back(new Menu("Build & Run         Ctrl+Shift+B", this->menubar.font, build_and_run_adept_project, this));

    message = NULL;
    double previousTime = glfwGetTime();
    int frameCount = 0;

    if(argc > 1) for(int i = 1; i != argc; i++){
        this->openEditor(std::string(argv[i]));
    }

    global_background_input.should_close.store(false);
    global_background_input.updated.store(false);
    global_background_output.updated.store(false);
    global_background_thread = new std::thread(background);

    double lastFrameTime = 0.0;

    while(!glfwWindowShouldClose(this->window)){
        double currentTime = glfwGetTime();
        frameCount++;

        this->settings.hidden.delta = (currentTime - lastFrameTime) * 60.0;
        
        if (currentTime - previousTime >= 1.0){
            // Display the frame count here any way you want.
            if(this->settings.ide_debug_fps)
                glfwSetWindowTitle(this->window, to_str(frameCount).c_str());

            frameCount = 0;
            previousTime = currentTime;
        }

        lastFrameTime = currentTime;

        // Handle user input & update
        this->handleInput();

        Editor *currentEditor = this->getCurrentEditor();
        
        // Prepare render
        glClearColor(0.07, 0.09, 0.1, 1.0f);
        //glClearColor(1.0, 1.0, 1.0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwGetFramebufferSize(this->window, &this->width, &this->height);
        glViewport(0, 0, this->width, this->height);
        this->projectionMatrix.ortho(0.0f, this->width, this->height, 0.0f, -1.0f, 1.0f);

        // Resize editors to new window size
        for(Editor *editor : this->editors){
            editor->resize(this->width, this->height - 32 - 20);
        }

        // Render menubar
        menubar.render(this->projectionMatrix, this->fontShader, this->solidShader, (float) this->width);

        // Render filenames and current editor
        if(currentEditor){
            this->renderEditorFilenames();
            currentEditor->render(this->projectionMatrix, this->fontShader, this->solidShader);
        } else if(this->settings.ide_emblem){
            // Render emblem
            this->shader->bind();
            this->shader->giveMatrix4f("projection_matrix", this->projectionMatrix);
            this->transformationMatrix.translateFromIdentity(this->width / 2 - 128.0f, this->height / 2 - 128.0f, 0.0f);
            this->shader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
            renderModel(this->emblemModel);
        }

        if(global_background_output.updated.load()){
            global_background_output.mutex.lock();
            // Use data that was processed in the background
            
            global_background_output.updated.store(false);
            global_background_output.mutex.unlock();
        }

        if(this->rootWatcher.changeOccured()){
            this->loadSettings();

            // Update stuff that might've been changed
            if(this->editors.size() != 0){
                this->menubar.underlineTab(this->currentEditorIndex);
            }
        }

        if(this->message){
            this->message->update(this->width, this->settings.hidden.delta);
            this->message->render(projectionMatrix, fontShader, solidShader, fontTexture, width, height);
            if(this->message->shouldClose(this->width)){
                delete this->message;
                this->message = NULL;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete message;

    global_background_input.should_close.store(true);
    global_background_thread->join();
    delete global_background_thread;

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void AdeptIDE::handleInput(){
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    Editor *currentEditor = this->getCurrentEditor();

    if(currentEditor){
        if(!this->mouseReleased && distance(mouseX + currentEditor->getNetXOffset(), mouseY + currentEditor->getNetYOffset(),
                    this->mouseDownX + this->mouseDownNetXOffset , this->mouseDownY + this->mouseDownNetYOffset) > 5.0f
            
            /*&& currentEditor->xOffset < mouseX*/ && currentEditor->yOffset < mouseY){
            currentEditor->moveCaretToPosition(this->mouseDownCaretPosition);
            this->startSelection();
            this->moveCaret(mouseX, mouseY);
            this->endSelection();
        }
    }

    this->update();
}

void AdeptIDE::update(){
    this->menubar.update();
}

void AdeptIDE::renderEditorFilenames(){
    this->transformationMatrix.translateFromIdentity(8.0f, 26.0f, 0.9);

    size_t index = 0;
    for(Editor *editor : this->editors){
        if(this->settings.editor_icons){
            this->shader->bind();
            this->shader->giveMatrix4f("projection_matrix", this->projectionMatrix);
            this->shader->giveMatrix4f("transformation_matrix", this->transformationMatrix);

            switch(editor->getFileType()){
            case PLAIN_TEXT: renderModel(this->plainTextModel); break;
            case ADEPT:      renderModel(this->adeptModel); break;
            case JAVA:       renderModel(this->javaModel);  break;
            case HTML:       renderModel(this->htmlModel);  break;
            }

            this->transformationMatrix.translate(24.0f, 0.0f, 0.0f);
        }

        this->fontShader->bind();
        this->fontShader->giveMatrix4f("projection_matrix", this->projectionMatrix);
        this->fontShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
        this->fontShader->giveFloat("width", 0.4f);
        this->fontShader->giveFloat("edge", 0.4f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->fontTexture->getID());

        if(editor->hasFilenameModel) editor->getFilenameModel()->draw();
        
        float advance = (editor->displayFilename.length() * (0.17f * this->font.mono_character_width)) + 16.0f;
        this->transformationMatrix.translate(advance, 0.0f, 0.0f);
        index++;
    }
}

void AdeptIDE::loadSettings(){
    this->settings.loadFromFile(this->root + "settings.json");
}

void AdeptIDE::openFile(){
    std::vector<std::string> filenames;
    if(!openMultipleFileDialog(this->window, filenames)) return;

    for(const std::string& filename : filenames)
        this->openEditor(filename);
}

void AdeptIDE::openEditor(const std::string& filename){
    Editor *newEditor = this->addEditor();

    newEditor->filename = filename;
    newEditor->updateFilenameModel();
    newEditor->loadTextFromFile(filename);

    this->updateTitle();
    this->setCurrentEditor(this->editors.size() - 1);
}

void AdeptIDE::newFile(){
    this->addEditor();
    this->updateTitle();

    // HACK: Set stuff for new file
    Editor *newEditor = this->editors[this->editors.size() - 1];
    newEditor->type(this->settings.editor_default_text);
    newEditor->moveCaretToPosition(this->settings.editor_default_position <= this->settings.editor_default_text.length() ? this->settings.editor_default_position : 0);
}

Editor* AdeptIDE::getCurrentEditor(){
    if(this->editors.size() == 0) return NULL;

    return this->editors[this->currentEditorIndex];
}

Editor* AdeptIDE::addEditor(){
    Editor *editor = new Editor();
    editor->load(&this->settings, &this->font, this->fontTexture, this->width, this->height);
    editor->setOffset(4.0f, 20.0f + 32.0f + 4.0f);
    editor->setFileType(this->settings.editor_default_language);
    editor->setSyntaxColorPalette(this->settings.editor_default_theme);
    this->editors.push_back(editor);
    this->setCurrentEditor(this->editors.size() - 1);
    return editor;
}

void AdeptIDE::removeEditor(size_t index){
    if(index < 0 || index > this->editors.size() - 1) return;

    delete this->editors[index];
    this->editors.erase(this->editors.begin() + index);
    if(index < this->currentEditorIndex || this->currentEditorIndex == this->editors.size()){
        this->setCurrentEditor(currentEditorIndex - 1);
    } else {
        this->setCurrentEditor(currentEditorIndex);
    }
}

void AdeptIDE::removeCurrentEditor(){
    if(this->editors.size() == 0) return;
    this->removeEditor(this->currentEditorIndex);
    this->updateTitle();
}

void AdeptIDE::moveToPreviousEditorTab(){
    if(this->editors.size() == 0) return;

    if(this->currentEditorIndex == 0){
        this->setCurrentEditor(this->editors.size() - 1);
        this->updateTitle();
        return;
    }

    this->setCurrentEditor(this->currentEditorIndex - 1);
    this->updateTitle();
}

void AdeptIDE::moveToNextEditorTab(){
    if(this->editors.size() == 0) return;

    if(this->currentEditorIndex == this->editors.size() - 1){
        this->setCurrentEditor(0);
        this->updateTitle();
        return;
    }

    this->setCurrentEditor(this->currentEditorIndex + 1);
    this->updateTitle();
}

void AdeptIDE::updateTitle(){
    Editor *editor = this->getCurrentEditor();
    
    if(editor == NULL){
        glfwSetWindowTitle(this->window, "AdeptIDE");
    } else if(editor->filename.length() == 0){
        glfwSetWindowTitle(this->window, "AdeptIDE - Untitled");
    } else {
        std::string title = "AdeptIDE - " + editor->filename;
        glfwSetWindowTitle(this->window, title.c_str());
    }
}

void AdeptIDE::setCurrentEditor(size_t index){
    if(index >= this->editors.size()){
        index = this->editors.size() - 1;
    }

    this->menubar.underlineTab(index);
    this->currentEditorIndex = index;
}

void AdeptIDE::scrollDown(int lineCount){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->scrollDown(lineCount);
}

void AdeptIDE::scrollUp(int lineCount){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->scrollUp(lineCount);
}

void AdeptIDE::pageUp(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->pageUp();
}

void AdeptIDE::pageDown(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->pageDown();
}

void AdeptIDE::selectAll(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->selectAll();
}

void AdeptIDE::selectLine(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->selectLine();
}

void AdeptIDE::duplicateCaretUp(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->duplicateCaretUp();
}

void AdeptIDE::duplicateCaretDown(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->duplicateCaretDown();
}

void AdeptIDE::type(const std::string& text){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->type(text);
}

void AdeptIDE::type(char character){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->type(character);
}

void AdeptIDE::typeBlock(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->typeBlock();
}

void AdeptIDE::typeExpression(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->typeExpression();
}

void AdeptIDE::typeArrayAccess(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->typeArrayAccess();
}

void AdeptIDE::typeString(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->typeString();
}

void AdeptIDE::typeCString(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->typeCString();
}

void AdeptIDE::backspace(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->backspace();
}

void AdeptIDE::del(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->del();
}

void AdeptIDE::smartBackspace(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->smartBackspace();
}

void AdeptIDE::smartDel(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->smartDel();
}

void AdeptIDE::backspaceLine(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->backspaceLine();
}

void AdeptIDE::delLine(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->delLine();
}

void AdeptIDE::startSelection(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->startSelection();
}

void AdeptIDE::endSelection(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->endSelection();
}

void AdeptIDE::destroySelection(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->destroySelection();
}

void AdeptIDE::deleteSelected(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->deleteSelected();
}

void AdeptIDE::copySelected(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->copySelected(this->window);
}

void AdeptIDE::cutSelected(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->cutSelected(this->window);
}

void AdeptIDE::paste(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->paste(this->window);
}

void AdeptIDE::tab(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->tab();
}

void AdeptIDE::nextLine(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->nextLine();
}

void AdeptIDE::nextPrecedingLine(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->nextPrecedingLine();
}

void AdeptIDE::finishSuggestion(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->finishSuggestion();
}

void AdeptIDE::moveCaret(double xpos, double ypos){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaret(xpos, ypos);
}

void AdeptIDE::moveCaretLeft(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaretLeft();
}

void AdeptIDE::moveCaretRight(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaretRight();
}

void AdeptIDE::moveCaretUp(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaretUp();
}

void AdeptIDE::moveCaretDown(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaretDown();
}

void AdeptIDE::moveCaretBeginningOfLine(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaretBeginningOfLine();
}

void AdeptIDE::moveCaretEndOfLine(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaretEndOfLine();
}

void AdeptIDE::moveCaretBeginningOfWord(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaretBeginningOfWord();
}

void AdeptIDE::moveCaretEndOfWord(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaretEndOfWord();
}

void AdeptIDE::moveCaretOutside(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaretOutside();
}

void AdeptIDE::moveCaretBeginningOfSubWord(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaretBeginningOfSubWord();
}

void AdeptIDE::moveCaretEndOfSubWord(){
    Editor *currentEditor = this->getCurrentEditor();
    if(currentEditor) currentEditor->moveCaretEndOfSubWord();
}

void AdeptIDE::saveFile(){
    Editor *currentEditor = this->getCurrentEditor();

    if(currentEditor){
        if(currentEditor->filename == ""){
            if(!saveFileDialog(this->window, currentEditor->filename)) return;
            this->updateTitle();
            currentEditor->updateFilenameModel();
            this->setCurrentEditor(this->currentEditorIndex);
        }
        currentEditor->saveFile();
    }
}

void AdeptIDE::createMessage(const std::string& message, double seconds){
    delete this->message;
    this->message = new Message(message, &font, seconds, width, height);
}

void scroll_callback(GLFWwindow *window, double xOffset, double yOffset){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(glfwGetWindowUserPointer(window));

    if(yOffset < -0.9){
        adeptide->scrollDown(8);
    } else if(yOffset > 0.9){
        adeptide->scrollUp(8);
    }
}

void character_callback(GLFWwindow *window, unsigned int codepoint){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(glfwGetWindowUserPointer(window));
    adeptide->type(codepoint);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(glfwGetWindowUserPointer(window));

    bool input = (action == GLFW_PRESS || action == GLFW_REPEAT);

    if(glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS && input){
        if(glfwGetKey(window, GLFW_KEY_SPACE)){
            adeptide->finishSuggestion();
            return;
        }

        quicktype(adeptide, key);
        return;
    }

    if(key == GLFW_KEY_COMMA && input && mods & GLFW_MOD_CONTROL){
        settings_menu(adeptide);
    }
    
    if(key == GLFW_KEY_BACKSPACE && input){
        if(mods & GLFW_MOD_CONTROL && mods & GLFW_MOD_SHIFT)
            adeptide->smartBackspace();
        else if(mods & GLFW_MOD_SHIFT)
            adeptide->backspaceLine();
        else
            adeptide->backspace();
        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_DELETE && input){
        if(mods & GLFW_MOD_CONTROL && mods & GLFW_MOD_SHIFT)
            adeptide->smartDel();
        else if(mods & GLFW_MOD_SHIFT)
            adeptide->delLine();
        else
            adeptide->del();
        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_ENTER && input){
        if(mods & GLFW_MOD_SHIFT)
            adeptide->nextLine();
        else if(mods & GLFW_MOD_CONTROL)
            adeptide->nextPrecedingLine();
        else
            adeptide->type('\n');
        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_TAB && input){
        adeptide->tab();
        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE){
        adeptide->endSelection();
    }

    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        Editor *editor = adeptide->getCurrentEditor();

        if(editor){
            if(editor->hasSelection()){
                adeptide->destroySelection();
            } else {
                editor->deleteAdditionalCarets();
            }
        }

        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_LEFT && input){
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            adeptide->startSelection();

        if(mods & GLFW_MOD_CONTROL) adeptide->moveCaretBeginningOfWord();
        else if(mods & GLFW_MOD_ALT) adeptide->moveCaretBeginningOfSubWord();
        else adeptide->moveCaretLeft();

        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_RIGHT && input){
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            adeptide->startSelection();

        if(mods & GLFW_MOD_CONTROL) adeptide->moveCaretEndOfWord();
        else if(mods & GLFW_MOD_ALT) adeptide->moveCaretEndOfSubWord();
        else adeptide->moveCaretRight();

        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_UP && input){
        if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            adeptide->scrollUp(8);
        } else if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
            adeptide->duplicateCaretUp();
        } else {
            if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                adeptide->startSelection();

            adeptide->moveCaretUp();

            adeptide->menubar.loseFocus();
        }
    }

    if(key == GLFW_KEY_DOWN && input){
        if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            adeptide->scrollDown(8);
        } else if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
            adeptide->duplicateCaretDown();
        } else {
            if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                adeptide->startSelection();

            adeptide->moveCaretDown();

            adeptide->menubar.loseFocus();
        }
    }

    if(key == GLFW_KEY_HOME && input){
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            adeptide->startSelection();

        adeptide->moveCaretBeginningOfLine();

        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_END && input){
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            adeptide->startSelection();

        adeptide->moveCaretEndOfLine();

        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_PAGE_UP && input){
        if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            adeptide->moveToPreviousEditorTab();
        else {
            if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                adeptide->startSelection();
            adeptide->pageUp();
        }

        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_PAGE_DOWN && input){
        if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            adeptide->moveToNextEditorTab();
        else {
            if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                adeptide->startSelection();
            adeptide->pageDown();
        }

        adeptide->menubar.loseFocus();
    }

    if(mods & GLFW_MOD_CONTROL && input){
        switch(key){
        case GLFW_KEY_1:
            if(mods & GLFW_MOD_SHIFT) theme_visual_studio(adeptide);
            else language_adept(adeptide);
            break;
        case GLFW_KEY_2:
            if(mods & GLFW_MOD_SHIFT) theme_tropical_ocean(adeptide);
            else language_java(adeptide);
            break;
        case GLFW_KEY_3:
            if(mods & GLFW_MOD_SHIFT) theme_island_campfire(adeptide);
            else language_html(adeptide);
            break;
        case GLFW_KEY_4:
            if(mods & GLFW_MOD_SHIFT) theme_one_dark(adeptide);
            break;
        case GLFW_KEY_A:
            adeptide->selectAll();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_L:
            adeptide->selectLine();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_O:
            adeptide->openFile();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_N:
            if(mods & GLFW_MOD_SHIFT) open_playground_menu(adeptide);
            else adeptide->newFile();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_P:
            adeptide->createMessage("This is a test message with some text\nWe can put whatever in here", 3.0);
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_S:
            adeptide->saveFile();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_C:
            adeptide->copySelected();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_X:
            adeptide->cutSelected();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_V:
            adeptide->paste();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_W:
            adeptide->removeCurrentEditor();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_B:
            if(mods & GLFW_MOD_SHIFT) build_and_run_adept_project(adeptide);
            else                      build_adept_project(adeptide);
            break;
        case GLFW_KEY_LEFT_BRACKET:
            if(mods & GLFW_MOD_SHIFT) adeptide->typeBlock();
            else                      adeptide->typeArrayAccess();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_9:
            if(mods & GLFW_MOD_SHIFT) adeptide->typeExpression();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_APOSTROPHE:
            if(mods & GLFW_MOD_SHIFT) adeptide->typeString();
            else adeptide->typeCString();
            adeptide->menubar.loseFocus();
            break;
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT){
        if(action == GLFW_PRESS){
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            handle_left_click(adeptide, xpos, ypos);
        } else if(action == GLFW_RELEASE){
            adeptide->mouseReleased = true;
        }
    }
}

void handle_left_click(AdeptIDE *adeptide, double xpos, double ypos){
    if(adeptide->menubar.leftClick(xpos, ypos, &adeptide->currentEditorIndex)){
        // Click menu
    } else {
        adeptide->menubar.loseFocus();

        Editor *editor = adeptide->getCurrentEditor();
        if(editor == NULL) return;

        adeptide->moveCaret(xpos, ypos);
        adeptide->mouseDownX = xpos;
        adeptide->mouseDownY = ypos;
        adeptide->mouseDownNetXOffset = editor->getNetXOffset();
        adeptide->mouseDownNetYOffset = editor->getNetYOffset();
        adeptide->mouseDownCaretPosition = editor->getCaretPosition();
        adeptide->mouseReleased = false;
    }
}

void drop_callback(GLFWwindow *window, int count, const char **paths){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(glfwGetWindowUserPointer(window));
    for(int i = 0; i != count; i++) adeptide->openEditor(paths[i]);
}

void file_menu(void *data){
    DropdownMenu *dropdownMenu = static_cast<DropdownMenu*>(data);
    dropdownMenu->isOpen = true;
}

void view_menu(void *data){
    DropdownMenu *dropdownMenu = static_cast<DropdownMenu*>(data);
    dropdownMenu->isOpen = true;
}

void new_file_menu(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->newFile();
    adeptide->menubar.loseFocus();
}

void open_playground_menu(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->openEditor(adeptide->root + "playground/playground.adept");
    adeptide->menubar.loseFocus();
}

void open_file_menu(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->openFile();
    adeptide->menubar.loseFocus();
}

void save_file_menu(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->saveFile();
    adeptide->menubar.loseFocus();
}

void close_file_menu(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->removeCurrentEditor();
    adeptide->menubar.loseFocus();
}

void settings_menu(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->openEditor(adeptide->root + "settings.json");
    adeptide->menubar.loseFocus();
}

void quit_menu(void *data){
    GLFWwindow *window = static_cast<GLFWwindow*>(data);
    glfwSetWindowShouldClose(window, true);
}

void select_all_menu(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->selectAll();
}

void language_plain(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    Editor *editor = adeptide->getCurrentEditor();
    if(editor){
        editor->setFileType(PLAIN_TEXT);
        adeptide->menubar.loseFocus();
    }
}

void language_adept(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    Editor *editor = adeptide->getCurrentEditor();
    if(editor){
        editor->setFileType(ADEPT);
        adeptide->menubar.loseFocus();
    }
}

void language_java(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    Editor *editor = adeptide->getCurrentEditor();
    if(editor){
        editor->setFileType(JAVA);
        adeptide->menubar.loseFocus();
    }
}

void language_html(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    Editor *editor = adeptide->getCurrentEditor();
    if(editor){
        editor->setFileType(HTML);
        adeptide->menubar.loseFocus();
    }
}

void theme_visual_studio(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    Editor *editor = adeptide->getCurrentEditor();
    if(editor){
        editor->setSyntaxColorPalette(SyntaxColorPalette::Defaults::VISUAL_STUDIO);
        adeptide->menubar.loseFocus();
    }
}

void theme_fruit_smoothie(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    Editor *editor = adeptide->getCurrentEditor();
    if(editor){
        editor->setSyntaxColorPalette(SyntaxColorPalette::Defaults::FRUIT_SMOOTHIE);
        adeptide->menubar.loseFocus();
    }
}

void theme_tropical_ocean(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    Editor *editor = adeptide->getCurrentEditor();
    if(editor){
        editor->setSyntaxColorPalette(SyntaxColorPalette::Defaults::TROPICAL_OCEAN);
        adeptide->menubar.loseFocus();
    }
}

void theme_island_campfire(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    Editor *editor = adeptide->getCurrentEditor();
    if(editor){
        editor->setSyntaxColorPalette(SyntaxColorPalette::Defaults::ISLAND_CAMPFIRE);
        adeptide->menubar.loseFocus();
    }
}

void theme_one_dark(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    Editor *editor = adeptide->getCurrentEditor();
    if(editor){
        editor->setSyntaxColorPalette(SyntaxColorPalette::Defaults::ONE_DARK);
        adeptide->menubar.loseFocus();
    }
}


void selection_menu(void *data){
    DropdownMenu *dropdownMenu = static_cast<DropdownMenu*>(data);
    dropdownMenu->isOpen = true;
}

void build_menu(void *data){
    DropdownMenu *dropdownMenu = static_cast<DropdownMenu*>(data);
    dropdownMenu->isOpen = true;
}

void build_adept_project(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    Editor *editor = adeptide->getCurrentEditor();
    if(editor){
        if(editor->filename == ""){
            adeptide->createMessage("File must be saved in order to compile", 2.0);
        }
        else if(system( (adeptide->settings.adept_compiler + " \"" + editor->filename + "\" > \"" + adeptide->root + "\"adept.log 2>&1").c_str() )){
            std::ifstream stream(adeptide->root + "adept.log");
            std::string notes;

            if(stream.is_open()){
                std::stringstream buffer;
                buffer << stream.rdbuf();
                notes = buffer.str();
                stream.close();

                if(notes.length() > 0 && notes[notes.length() - 1] == '\n'){
                    notes.resize(notes.length() - 1);
                }
            }

            adeptide->createMessage(notes.length() ? "Failed to compile:\n" + notes : "Failed to compile", 3.0);
        } else {
            adeptide->createMessage("Build Successful", 1.0);
        }
    }
    adeptide->menubar.loseFocus();
}

void build_and_run_adept_project(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    Editor *editor = adeptide->getCurrentEditor();
    if(editor){
        if(editor->filename == ""){
            adeptide->createMessage("File must be saved in order to compile", 2.0);
        }
        else if(system( (adeptide->settings.adept_compiler + " -e \"" + editor->filename + "\"").c_str() )){
            std::ifstream stream(adeptide->root + "adept.log");
            std::string notes;

            if(stream.is_open()){
                std::stringstream buffer;
                buffer << stream.rdbuf();
                notes = buffer.str();
                stream.close();

                if(notes.length() > 0 && notes[notes.length() - 1] == '\n'){
                    notes.resize(notes.length() - 1);
                }
            }

            adeptide->createMessage(notes.length() ? "Failed to compile:\n" + notes : "Failed to compile", 3.0);
        } else {
            adeptide->createMessage("Build Successful", 1.0);
        }
    }
    adeptide->menubar.loseFocus();
}

Model* makeSquareModel(Texture *texture, float size){
    std::vector<float> vertices;
    std::vector<float> textureCoords;
    std::vector<unsigned int> indices;

    vertices.push_back(  0.0f);
    vertices.push_back(size);
    vertices.push_back(  0);
    vertices.push_back(  0.0f);
    vertices.push_back(  0.0f);
    vertices.push_back(  0);
    vertices.push_back(size);
    vertices.push_back(  0.0f);
    vertices.push_back(  0.0f);
    vertices.push_back(size);
    vertices.push_back(size);
    vertices.push_back(  0.0f);

    textureCoords.push_back(0.0f);
    textureCoords.push_back(1.0f);
    textureCoords.push_back(0.0f);
    textureCoords.push_back(0.0f);
    textureCoords.push_back(1.0f);
    textureCoords.push_back(0.0f);
    textureCoords.push_back(1.0f);
    textureCoords.push_back(1.0f);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(3);
    indices.push_back(3);
    indices.push_back(1);
    indices.push_back(2);

    VAO *vao = new VAO();
    vao->bind();

    VBO *verticesVBO = new VBO(VBOContentType::VERTICES, vertices);
    vao->addVBO(verticesVBO);

    VBO *textureCoordsVBO = new VBO(VBOContentType::TEXTURE_COORDS, textureCoords);
    vao->addVBO(textureCoordsVBO);

    EBO *ebo = new EBO(indices);

    return new Model(vao, ebo, texture);
}

double distance(double x1, double y1, double x2, double y2){
    double xdiff = x1 - x2;
    double ydiff = y1 - y2;
    return sqrt(xdiff * xdiff + ydiff * ydiff);
}
