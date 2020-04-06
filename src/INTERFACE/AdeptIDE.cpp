
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
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#endif

AdeptIDE::AdeptIDE() : AdeptIDEAssets() {
    this->fontTexture = NULL;

    this->window = NULL;
    this->mouseReleased = true;
    this->mouseDownX = 0.0f;
    this->mouseDownY = 0.0f;
    this->currentEditorIndex = 0;

    this->shader                = NULL;
    this->fontShader            = NULL;
    this->solidShader           = NULL;

    this->message = NULL;
    this->explorer = NULL;
    this->terminal = NULL;
    this->fileLooker = NULL;
    this->lineNavigator = NULL;
}

AdeptIDE::~AdeptIDE(){
    delete fontTexture;
    delete this->shader;
    delete this->fontShader;
    delete this->solidShader;

    for(GenericEditor* editor : this->editors){
        editor->free();
        delete editor;
    }

    delete this->message;
    delete this->explorer;
    delete this->terminal;
    delete this->fileLooker;
    delete this->lineNavigator;
}

int AdeptIDE::main(int argc, const char **argv){
    if(!glfwInit()){
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }

    // Determine root directory of executable
    {
        char location[1024];

        #ifdef _WIN32
        GetModuleFileNameA(NULL, location, 512);
        #elif defined(__APPLE__)
        {
            uint32_t size = sizeof(location);
            if (_NSGetExecutablePath(location, &size) != 0){
                std::cerr << "EXTERNAL ERROR: Executable path is too long!\n" << std::endl;
                return 1;
            }
        }
        #else
        if (argv == NULL || argv[0] == NULL || strcmp(argv[0], "") == 0)
        {
            std::cerr << "EXTERNAL ERROR: Compiler was invoked with NULL or empty argv[0]\n" << std::endl;
            return 1;
        }
        char *l = filename_absolute(argv[0]);
        strcpy(location, l);
        free(l);
        #endif

        char *absolute_filename = filename_absolute(location);
        char *absolute_path = filename_path(absolute_filename);
        this->root = absolute_path;
        this->assetsFolder = this->root + "assets/";
        free(absolute_path);
        free(absolute_filename);
    }

    // Load settings
    this->rootWatcher.target(this->root);
    this->loadSettings();

    const bool fullscreen = false;
    glfwWindowHint(GLFW_MAXIMIZED, this->settings.ide_default_maximized);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if(fullscreen){
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *vidmode = glfwGetVideoMode(monitor);
        this->width = vidmode->width;
        this->height = vidmode->height;
        this->window = glfwCreateWindow(this->width, this->height, "AdeptIDE", monitor, NULL);
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
    this->font.load(this->assetsFolder + FONT_NAME + ".fnt");
    this->fontTexture           = new Texture(this->assetsFolder + FONT_NAME + ".png", TextureLoadOptions::ALPHA_BLEND);
    this->shader                = new Shader(this->assetsFolder + "vertex.glsl",       this->assetsFolder + "fragment.glsl",       {"position", "uvs"});
    this->fontShader            = new Shader(this->assetsFolder + "font_vertex.glsl",  this->assetsFolder + "font_fragment.glsl",  {"position", "uvs"});
    this->solidShader           = new Shader(this->assetsFolder + "solid_vertex.glsl", this->assetsFolder + "solid_fragment.glsl", {"position"});
    this->loadAdeptIDEAssets(this->assetsFolder);

    this->menubar.load(&this->settings, &this->font, this->fontTexture, &this->editors, !this->settings.explorer_default_collapse);

    this->menubar.addMenu("FILE",      &file_menu,      NULL);
    this->menubar.addMenu("EDIT",      NULL,            NULL);
    this->menubar.addMenu("VIEW",      &view_menu,      NULL);
    this->menubar.addMenu("SELECTION", &selection_menu, NULL);
    this->menubar.addMenu("BUILD",     &build_menu,     NULL);
    this->menubar.addMenu("EXECUTE",   NULL,            NULL);
    this->menubar.addMenu("HELP",      &help_menu,      NULL);

    DropdownMenu *newFileDropdown = new DropdownMenu(&this->font, 8.0f + 256.0f + 16.0f * 3, 20.0f, 32);
    Menu *newFileMenu = new Menu("New File                       >", this->menubar.font, NULL, newFileDropdown);
    newFileMenu->dropdownMenu = newFileDropdown;
    newFileDropdown->menus.push_back(new Menu("Adept File                Ctrl+N", this->menubar.font, new_adept_file, this));
    newFileDropdown->menus.push_back(new Menu("Java File                       ", this->menubar.font, new_java_file, this));
    newFileDropdown->menus.push_back(new Menu("Plain Text File                 ", this->menubar.font, new_plain_text_file, this));
    newFileDropdown->menus.push_back(new Menu("Painting                        ", this->menubar.font, new_painting_file, this));

    this->menubar.menus[0]->dropdownMenu = new DropdownMenu(&this->font, 4.0f, 20.0f, 32);
    this->menubar.menus[0]->data = this->menubar.menus[0]->dropdownMenu;
    this->menubar.menus[0]->dropdownMenu->menus.push_back(newFileMenu);
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Open File                 Ctrl+O", this->menubar.font, &open_file_menu, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Open Folder         Ctrl+Shift+O", this->menubar.font, &open_folder_menu, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Open Playground     Ctrl+Shift+N", this->menubar.font, &open_playground_menu, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Save File                 Ctrl+S", this->menubar.font, &save_file_menu, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Save File As        Ctrl+Shift+S", this->menubar.font, NULL, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Save all                  Ctrl+T", this->menubar.font, NULL, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Close File                Ctrl+W", this->menubar.font, &close_file_menu, this));
    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu("Settings              Ctrl+Comma", this->menubar.font, &settings_menu, this));

    #ifdef __APPLE__
    std::string quitString = "Quit                       Cmd+Q";
    #else
    std::string quitString = "Quit                      Alt+F4";
    #endif

    this->menubar.menus[0]->dropdownMenu->menus.push_back(new Menu(quitString, this->menubar.font, &quit_menu, this->window));

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
    this->menubar.menus[2]->dropdownMenu->menus.push_back(new Menu("Maximize                         ", this->menubar.font, maximize, this));

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

    selectionDropDownX = 8.0f;
    for(int q = 0; q != 6; q++)
        selectionDropDownX += 16.0f +this->menubar.menus[q]->textWidth;
    DropdownMenu *helpMenu = new DropdownMenu(&this->font, selectionDropDownX, 20.0f, 32);
    this->menubar.menus[6]->dropdownMenu = helpMenu;
    this->menubar.menus[6]->data = helpMenu;
    this->menubar.menus[6]->dropdownMenu->menus.push_back(new Menu("About", this->menubar.font, about_menu, this));

    this->fileLooker = new FileLooker();
    this->fileLooker->load(&this->settings, &this->font, this->fontTexture, 512, 32); // 320 h

    this->lineNavigator = new LineNavigator();
    this->lineNavigator->load(&this->settings, &this->font, this->fontTexture, 512, 32);

    this->message = NULL;
    this->explorer = new Explorer();
    this->explorer->load(&this->settings, &this->font, this->fontTexture, 256, this->height, this->fileLooker);

    this->terminal = new Terminal();
    this->terminal->load(&this->settings, &this->font, this->fontTexture, this->width, 256);

    double previousTime = glfwGetTime();
    int frameCount = 0;

    if(argc > 1) for(int i = 1; i != argc; i++){
        #ifdef __APPLE__
        // Ignore launch signal
        if(strncmp(argv[i], "-psn", 4) == 0) continue;
        #endif // __APPLE__
        
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

        if(this->settings.hidden.fastForward){
            this->settings.hidden.delta = 1.0;
            this->settings.hidden.fastForward = false;
        } else {
            this->settings.hidden.delta = (currentTime - lastFrameTime) * 60.0;
        }

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

        GenericEditor *currentEditor = this->getCurrentEditor();

        if(TextEditor *textEditor = this->getCurrentEditorAsTextEditor()){
            if(textEditor->astMutex.try_lock()){

                switch(textEditor->astCreationResult){
                case AstCreationResultNothingNew:
                    break;
                case AstCreationResultFailure:
                    this->createMessage("Failed to update insight due to syntax error(s)", 3.0);
                    textEditor->astCreationResult = AstCreationResultNothingNew;
                    break;
                case AstCreationResultSuccess:
                    this->createMessage("Successfully updated insight", 2.0);
                    textEditor->astCreationResult = AstCreationResultNothingNew;
                    break;
                case AstCreationResultNotAdept:
                    this->createMessage("Can't gain insight into non-Adept source code", 3.0);
                    textEditor->astCreationResult = AstCreationResultNothingNew;
                    break;
                }

                textEditor->astMutex.unlock();
            }
        }

        // Prepare render
        glClearColor(0.07, 0.09, 0.1, 1.0f);
        //glClearColor(1.0, 1.0, 1.0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwGetFramebufferSize(this->window, &this->width, &this->height);
        glViewport(0, 0, this->width, this->height);
        this->projectionMatrix.ortho(0.0f, this->width, this->height, 0.0f, -1.0f, 1.0f);

        if(this->explorer) this->explorer->resize(this->height);
        if(this->terminal) this->terminal->resize(this->width);

        // Resize editors to new window size
        for(GenericEditor *editor : this->editors){
            TextEditor *textEditor = editor->asTextEditor();
            if(textEditor) textEditor->resize(this->width, this->height - 32 - 20);
        }

        // Render menubar
        menubar.render(this->projectionMatrix, this->fontShader, this->solidShader, (float) this->width);

        // Render filenames and current editor
        if(currentEditor){
            this->renderEditorFilenames();

            float offsetByExplorer = this->explorer ? this->explorer->containerX + 256.0f : 0.0f;
            float xOffset = offsetByExplorer + 4.0f;
            float yOffset = 20.0f + 32.0f + 4.0f;

            currentEditor->setOffset(xOffset, yOffset);
            currentEditor->render(this->projectionMatrix, this->shader, this->fontShader, this->solidShader, (AdeptIDEAssets*) this);
        } else if(this->settings.ide_emblem){
            // Render emblem
            this->shader->bind();
            this->shader->giveMatrix4f("projection_matrix", this->projectionMatrix);

            float offsetByExplorer = this->explorer ? this->explorer->containerX + 256.0f : 0.0f;
            this->transformationMatrix.translateFromIdentity((this->width - offsetByExplorer) / 2 - 128.0f + offsetByExplorer, this->height / 2 - 128.0f, -0.99f);
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
        }

        if(this->explorer){
            this->shader->bind();
            this->shader->giveMatrix4f("projection_matrix", this->projectionMatrix);
            this->transformationMatrix.translateFromIdentity(8.0f, 24.0f + 4.0f, 0.71f);
            this->shader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
            renderModel(this->explorerToggleModel);

            this->transformationMatrix.translate(32.0f, 0.0f, 0.0f);
            this->shader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
            renderModel(this->openFolderModel);

            this->transformationMatrix.translate(32.0f, 0.0f, 0.0f);
            this->shader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
            renderModel(this->treeModel);

            this->transformationMatrix.translate(32.0f, 0.0f, 0.0f);
            this->shader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
            renderModel(this->cdModel);

            this->explorer->render(projectionMatrix, shader, fontShader, solidShader, (AdeptIDEAssets*) this);
        }

        if(this->terminal){
            this->terminal->containerX = (this->explorer ? this->explorer->containerX : 0) + 256;
            this->terminal->render(projectionMatrix, shader, fontShader, solidShader, this->height, (AdeptIDEAssets*) this);
        }

        if(this->fileLooker){
            this->fileLooker->containerX = this->width / 2 - this->fileLooker->containerWidth / 2;
            this->fileLooker->containerY = 52;
            this->fileLooker->render(projectionMatrix, shader, fontShader, solidShader, (AdeptIDEAssets*) this);
        }

        if(this->lineNavigator){
            this->lineNavigator->containerX = this->width / 2 - this->fileLooker->containerWidth / 2;
            this->lineNavigator->containerY = 52;
            this->lineNavigator->render(projectionMatrix, shader, fontShader, solidShader, (AdeptIDEAssets*) this);
        }

        if(this->message){
            this->message->render(projectionMatrix, fontShader, solidShader, fontTexture, width, height);
            if(this->message->shouldClose(this->width)){
                delete this->message;
                this->message = NULL;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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

    if(this->explorer){
        this->explorer->update();
    }

    if(TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor()){
        if(!this->mouseReleased && distance(mouseX + currentTextEditor->getNetXOffset(), mouseY + currentTextEditor->getNetYOffset(),
                    this->mouseDownX + this->mouseDownNetXOffset , this->mouseDownY + this->mouseDownNetYOffset) > 5.0f
            
            /*&& currentTextEditor->xOffset < mouseX*/ && currentTextEditor->yOffset < mouseY){
            currentTextEditor->moveCaretToPosition(this->mouseDownCaretPosition);
            this->startSelection();
            this->moveCaret(mouseX, mouseY);
            this->endSelection();
        }

        this->update();
        return;
    }

    if(ImageEditor *currentImageEditor = this->getCurrentEditorAsImageEditor()){
        if(currentImageEditor->isDragging()){
            if(glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_LEFT)){
                currentImageEditor->updateDrag(mouseX, mouseY);
            } else {
                currentImageEditor->endDrag(mouseX, mouseY);
            }
        } else {
            if(glfwGetMouseButton(this->window, GLFW_MOUSE_BUTTON_LEFT)){
                currentImageEditor->startDrag(mouseX, mouseY);
            }
        }

        this->update();
        return;
    }

    this->update();
}

void AdeptIDE::update(){
    this->menubar.update(this->explorer->isVisible());
    if(this->message) this->message->update(this->width, this->settings.hidden.delta);
}

void AdeptIDE::renderEditorFilenames(){
    this->transformationMatrix.translateFromIdentity(this->menubar.filenamesOffsetX + 8.0f, 26.0f, 0.9);

    size_t index = 0;
    for(GenericEditor *editor : this->editors){
        if(this->settings.editor_icons){
            this->shader->bind();
            this->shader->giveMatrix4f("projection_matrix", this->projectionMatrix);
            this->shader->giveMatrix4f("transformation_matrix", this->transformationMatrix);

            switch(editor->getFileType()){
            case PLAIN_TEXT: renderModel(this->plainTextModel); break;
            case ADEPT:      renderModel(this->adeptModel);     break;
            case JAVA:       renderModel(this->javaModel);      break;
            case HTML:       renderModel(this->htmlModel);      break;
            case PAINTING:   renderModel(this->paintingModel);  break;
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

        TextModel *editorFilenameTextModel = editor->getFilenameModel();
        if(editorFilenameTextModel) editorFilenameTextModel->draw();
        
        float advance = (editor->displayFilename.length() * (FONT_SCALE * this->font.mono_character_width)) + 16.0f;
        this->transformationMatrix.translate(advance, 0.0f, 0.0f);
        index++;
    }
}

void AdeptIDE::loadSettings(){
    this->settings.loadFromFile(this->root + "settings.json");

    // Update stuff that might've been changed
    if(this->editors.size() != 0){
        this->menubar.underlineTab(this->currentEditorIndex);
    }

    // Refresh explorer
    if(this->explorer && this->explorer->refreshNodes())
        this->createMessage("Too many files in directory! Some files omitted!", 3.0);
}

void AdeptIDE::openFile(){
    this->settings.hidden.fastForward = true;

    std::vector<std::string> filenames;
    if(!openMultipleFileDialog(this->window, filenames)){
        return;
    }

    for(const std::string& filename : filenames)
        this->openEditor(filename);
    glfwSetTime(glfwGetTime());
    this->settings.hidden.delta = 0;
}

void AdeptIDE::openFolder(){
    this->settings.hidden.fastForward = true;

    if(this->explorer){
        std::string folder;

        if(openFolderDialog(this->window, folder)){
            if(this->explorer->setRootFolder(folder))
                this->createMessage("Too many files in directory! Some files omitted!", 3.0);
            this->explorer->setVisibility(true);
        }
    }
}

void AdeptIDE::openEditor(const std::string& filename){
    TextEditor *newEditor = this->addTextEditor();

    newEditor->filename = filename;
    newEditor->updateFilenameModel();
    newEditor->loadTextFromFile(filename);
    this->updateTitle();

    // Re-underline the tab because we changed the filename model
    if(this->editors.size() != 0)
        this->menubar.underlineTab(this->currentEditorIndex);

    this->settings.hidden.fastForward = true;
}

void AdeptIDE::newFile(FileType fileType){
    switch(fileType){
    case FileType::PAINTING: {
            this->addImageEditor();
            this->updateTitle();
        }
        break;
    default: {
            TextEditor *newEditor = this->addTextEditor();
            newEditor->setFileType(fileType);
            this->updateTitle();
            newEditor->type(this->settings.editor_default_text);
            newEditor->moveCaretToPosition(this->settings.editor_default_position <= this->settings.editor_default_text.length() ? this->settings.editor_default_position : 0);
        }
    }
}

GenericEditor* AdeptIDE::getCurrentEditor(){
    if(this->editors.size() == 0) return NULL;

    return this->editors[this->currentEditorIndex];
}

TextEditor* AdeptIDE::getCurrentEditorAsTextEditor(){
    if (this->editors.size() == 0) return NULL;
    return this->editors[this->currentEditorIndex]->asTextEditor();
}

ImageEditor* AdeptIDE::getCurrentEditorAsImageEditor(){
    if (this->editors.size() == 0) return NULL;
    return this->editors[this->currentEditorIndex]->asImageEditor();
}

TextEditor* AdeptIDE::addTextEditor(){
    float xOffset = this->explorer && this->explorer->isVisible() ? 256.0f + 4.0f : 4.0f;
    float yOffset = 20.0f + 32.0f + 4.0f;

    TextEditor *editor = new TextEditor();
    editor->load(&this->settings, &this->font, this->fontTexture, this->width, this->height);
    editor->snapCaretToPosition(xOffset, 0.0f);
    editor->setOffset(xOffset, yOffset);
    editor->setFileType(this->settings.editor_default_language);
    editor->setSyntaxColorPalette(this->settings.editor_default_theme);
    this->editors.push_back(editor);
    this->setCurrentEditor(this->editors.size() - 1);
    return editor;
}

ImageEditor* AdeptIDE::addImageEditor(){
    ImageEditor *editor = new ImageEditor();
    editor->load(&this->settings, &this->font, this->fontTexture, this->width, this->height);
    this->editors.push_back(editor);
    this->setCurrentEditor(this->editors.size() - 1);
    return editor;
}

void AdeptIDE::removeEditor(size_t index){
    if(index < 0 || index > this->editors.size() - 1) return;

    this->editors[index]->free();
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
    TextEditor *editor = this->getCurrentEditorAsTextEditor();

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

void AdeptIDE::updateInsight(){
    if(TextEditor *textEditor = this->getCurrentEditorAsTextEditor())
        textEditor->makeAst(true, true);
}

void AdeptIDE::scrollDown(int lineCount){
    if(this->explorer){
        double x, y;
        glfwGetCursorPos(window, &x, &y);

        if(this->explorer->scrollDownIfHovering(x, y, lineCount)) return;
    }

    if(this->terminal){
        double x, y;
        glfwGetCursorPos(window, &x, &y);

        int w, h;
        glfwGetFramebufferSize(this->window, &w, &h);
        
        if(this->terminal->scrollDownIfHovering(x, y, h, lineCount)) return;
    }

    if(TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor()){
        currentTextEditor->scrollDown(lineCount);
        return;
    }

    if(ImageEditor *currentImageEditor = this->getCurrentEditorAsImageEditor()){
        currentImageEditor->zoomOut(lineCount);
        return;
    }
}

void AdeptIDE::scrollUp(int lineCount){
    if(this->explorer){
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        
        if(this->explorer->scrollUpIfHovering(x, y, lineCount)) return;
    }

    if(this->terminal){
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        
        int w, h;
        glfwGetFramebufferSize(this->window, &w, &h);
        
        if(this->terminal->scrollUpIfHovering(x, y, h, lineCount)) return;
    }

    if(TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor()){
        currentTextEditor->scrollUp(lineCount);
        return;
    }

    if(ImageEditor *currentImageEditor = this->getCurrentEditorAsImageEditor()){
        currentImageEditor->zoomIn(lineCount);
    }
}

void AdeptIDE::pageUp(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->pageUp();
}

void AdeptIDE::pageDown(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->pageDown();
}

void AdeptIDE::selectAll(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->selectAll();
}

void AdeptIDE::selectLine(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->selectLine();
}

void AdeptIDE::duplicateCaretUp(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->duplicateCaretUp();
}

void AdeptIDE::duplicateCaretDown(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->duplicateCaretDown();
}

void AdeptIDE::type(const std::string& text){
    if(this->fileLooker && this->fileLooker->type(text)) return;
    if(this->lineNavigator && this->lineNavigator->type(text)) return;
    if(this->terminal && this->terminal->type(text)) return;

    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->type(text);
}

void AdeptIDE::type(char character){
    if(this->fileLooker && this->fileLooker->type(character)) return;
    if(this->lineNavigator && this->lineNavigator->type(character)) return;
    if(this->terminal && this->terminal->type(character)) return;

    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->type(character);
}

void AdeptIDE::typeBlock(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->typeBlock();
}

void AdeptIDE::typeExpression(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->typeExpression();
}

void AdeptIDE::typeArrayAccess(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->typeArrayAccess();
}

void AdeptIDE::typeString(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->typeString();
}

void AdeptIDE::typeCString(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->typeCString();
}

void AdeptIDE::backspace(){
    if(this->fileLooker && this->fileLooker->isVisible()){
        this->fileLooker->backspace();
        return;
    }

    if(this->lineNavigator && this->lineNavigator->isVisible()){
        this->lineNavigator->backspace();
        return;
    }
    
    if(this->terminal && this->terminal->isVisible()){
        this->terminal->backspace();
        return;
    }

    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->backspace();
}

void AdeptIDE::del(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->del();
}

void AdeptIDE::smartBackspace(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->smartBackspace();
}

void AdeptIDE::smartDel(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->smartDel();
}

void AdeptIDE::backspaceLine(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->backspaceLine();
}

void AdeptIDE::delLine(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->delLine();
}

void AdeptIDE::startSelection(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->startSelection();
}

void AdeptIDE::endSelection(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->endSelection();
}

void AdeptIDE::destroySelection(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->destroySelection();
}

void AdeptIDE::deleteSelected(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->deleteSelected();
}

void AdeptIDE::copySelected(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->copySelected(this->window);
}

void AdeptIDE::cutSelected(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->cutSelected(this->window);
}

void AdeptIDE::paste(){
    if(this->terminal && this->terminal->isVisible()){
        this->terminal->paste(this->window);
    } else if(TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor()){
        currentTextEditor->paste(this->window);
    }
}

void AdeptIDE::tab(){
    if(this->terminal && this->terminal->isVisible()){
        this->terminal->type('\t');
        return;
    }

    if(TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor()){
        currentTextEditor->tab();
        return;
    }
}

void AdeptIDE::nextLine(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->nextLine();
}

void AdeptIDE::nextPrecedingLine(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->nextPrecedingLine();
}

void AdeptIDE::finishSuggestion(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->finishSuggestion();
}

void AdeptIDE::moveCaret(double xpos, double ypos){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaret(xpos, ypos);
}

void AdeptIDE::moveCaretLeft(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaretLeft();
}

void AdeptIDE::moveCaretRight(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaretRight();
}

void AdeptIDE::moveCaretUp(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaretUp();
}

void AdeptIDE::moveCaretDown(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaretDown();
}

void AdeptIDE::moveCaretBeginningOfLine(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaretBeginningOfLine();
}

void AdeptIDE::moveCaretEndOfLine(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaretEndOfLine();
}

void AdeptIDE::moveCaretBeginningOfWord(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaretBeginningOfWord();
}

void AdeptIDE::moveCaretEndOfWord(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaretEndOfWord();
}

void AdeptIDE::moveCaretOutside(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaretOutside();
}

void AdeptIDE::moveCaretBeginningOfSubWord(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaretBeginningOfSubWord();
}

void AdeptIDE::moveCaretEndOfSubWord(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();
    if(currentTextEditor) currentTextEditor->moveCaretEndOfSubWord();
}

void AdeptIDE::saveFile(){
    TextEditor *currentTextEditor = this->getCurrentEditorAsTextEditor();

    if(currentTextEditor){
        if(currentTextEditor->filename == ""){
            if(!saveFileDialog(this->window, currentTextEditor->filename)) return;
            this->updateTitle();
            currentTextEditor->updateFilenameModel();
            this->setCurrentEditor(this->currentEditorIndex);
        }
        currentTextEditor->saveFile();

        #ifdef __APPLE__
        // HACK: kevent doesn't catch when we write to the file but it does capture when others
        // write to it for some reason. Because of that, we'll just reload the settings when
        // we save settings.json - Isaac, Dec 27 2018
        // TODO: Fix whatevers not right with kevent usage in FolderWatcher
        if(currentTextEditor->filename == this->root + "settings.json"){
            this->loadSettings();
        }
        #endif
    }
}

void AdeptIDE::runFile(){
    TextEditor *editor = this->getCurrentEditorAsTextEditor();
    if(editor == NULL) return;

    std::string target = filename_get_without_extension(editor->filename)
    #if _WIN32
    + ".exe";
    #else
    ;
    #endif // _WIN32

    if(access(target.c_str(), F_OK) == -1){
        // File doesn't exist
        this->createMessage("Couldn't find default executable:\n'" + target + "'", 3.0);
        return;
    }

    char cwd[512];
    getcwd(cwd, 512);

    chdir(filename_path(target).c_str());
    system(("\"" + target + "\"").c_str());
    chdir(cwd);
}

void AdeptIDE::lookForFile(){
    if(!this->fileLooker) return;
    if(this->lineNavigator) this->lineNavigator->setVisibility(false);
    
    this->fileLooker->toggleVisibility();
}

void AdeptIDE::gotoLine(){
    if(!this->fileLooker) return;
    if(this->fileLooker) this->fileLooker->setVisibility(false);

    this->lineNavigator->toggleVisibility();
}

void AdeptIDE::cdFile(){
    if(TextEditor *textEditor = this->getCurrentEditorAsTextEditor()){
        if(this->terminal){
            char *path = filename_path(textEditor->filename.c_str());
            this->terminal->type("cd \"" + std::string(path) + "\"\n");
            free(path);
        }
        return;
    }

    if(this->explorer && this->explorer->getFolderPath() != ""){
        this->terminal->type("cd \"" + this->explorer->getFolderPath() + "\"\n");
        return;
    }
}

void AdeptIDE::createMessage(const std::string& message, double seconds){
    delete this->message;
    this->message = new Message(message, &font, seconds, width, height);
}

void scroll_callback(GLFWwindow *window, double xOffset, double yOffset){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(glfwGetWindowUserPointer(window));

    double amount = adeptide->settings.ide_scroll_fixed ? 8.0 : fabs(yOffset);

    if(yOffset < -0.9){
        adeptide->scrollDown(amount * adeptide->settings.ide_scroll_multiplier);
    } else if(yOffset > 0.9){
        adeptide->scrollUp(amount * adeptide->settings.ide_scroll_multiplier);
    }
}

void character_callback(GLFWwindow *window, unsigned int codepoint){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(glfwGetWindowUserPointer(window));
    adeptide->type(codepoint);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(glfwGetWindowUserPointer(window));

    bool input = (action == GLFW_PRESS || action == GLFW_REPEAT);

    #ifdef __APPLE__
    #define QUICKTYPE_KEY GLFW_KEY_LEFT_SUPER
    #define CMDCTRL_MOD(a) (a & GLFW_MOD_SUPER || a & GLFW_MOD_CONTROL)
    #else
    #define QUICKTYPE_KEY GLFW_KEY_RIGHT_ALT
    #define CMDCTRL_MOD(a) (a & GLFW_MOD_CONTROL)
    #endif

    if(key == GLFW_KEY_COMMA && input && CMDCTRL_MOD(mods)){
        if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
        if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);
        settings_menu(adeptide);
        return;
    }

    if(adeptide->settings.ide_quicktype && glfwGetKey(window, QUICKTYPE_KEY) == GLFW_PRESS && input){
        if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS){
            adeptide->finishSuggestion();
            return;
        }

        quicktype(adeptide, key);
        return;
    }

    #ifdef __APPLE__
    if(key == GLFW_KEY_SLASH && input && mods & GLFW_MOD_SUPER){
    #else
    if(key == GLFW_KEY_SLASH && input && mods & GLFW_MOD_CONTROL){
    #endif
        if(adeptide->explorer){
            adeptide->explorer->toggleVisibility();
            if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
            if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);
        }
    }

    #ifdef __APPLE__
    if(key == GLFW_KEY_MINUS && input && (mods & GLFW_MOD_SUPER)){
    #else
    if(key == GLFW_KEY_MINUS && input && mods & GLFW_MOD_CONTROL){
    #endif
        if(adeptide->terminal){
            adeptide->terminal->toggleVisibility();
            if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
            if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);
        }
    }
    
    if(key == GLFW_KEY_BACKSPACE && input){
        if(CMDCTRL_MOD(mods) && mods & GLFW_MOD_SHIFT)
            adeptide->smartBackspace();
        else if(mods & GLFW_MOD_SHIFT)
            adeptide->backspaceLine();
        else
            adeptide->backspace();
        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_DELETE && input){
        if(CMDCTRL_MOD(mods) && mods & GLFW_MOD_SHIFT)
            adeptide->smartDel();
        else if(mods & GLFW_MOD_SHIFT)
            adeptide->delLine();
        else
            adeptide->del();
        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_ENTER && input){
        if(adeptide->fileLooker && adeptide->fileLooker->isVisible()){
            std::string found = adeptide->fileLooker->look();
            adeptide->fileLooker->setVisibility(false);
            if(found != ""){
                adeptide->openEditor(found);
            } else {
                adeptide->createMessage("No matches found", 1);
            }
        } else if(adeptide->lineNavigator && adeptide->lineNavigator->isVisible()){
            int line = adeptide->lineNavigator->getLineNumber();
            adeptide->lineNavigator->setVisibility(false);

            if(line == 0){
                adeptide->createMessage("Invalid line number", 1);
            } else if(TextEditor *currentTextEditor = adeptide->getCurrentEditorAsTextEditor()){
                currentTextEditor->gotoLine(line);
            }
        } else if(mods & GLFW_MOD_SHIFT)
            adeptide->nextLine();
        else if(CMDCTRL_MOD(mods))
            adeptide->nextPrecedingLine();
        else
            adeptide->type('\n');
        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_TAB && input){
        if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
        if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);
        adeptide->tab();
        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE){
        adeptide->endSelection();
    }

    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        if(adeptide->fileLooker && adeptide->fileLooker->isVisible()){
            adeptide->fileLooker->setVisibility(false);
        } else if(adeptide->lineNavigator && adeptide->lineNavigator->isVisible()){
            adeptide->lineNavigator->setVisibility(false);
        } else if(TextEditor *editor = adeptide->getCurrentEditorAsTextEditor()){
            if(editor->hasSelection()){
                adeptide->destroySelection();
            } else {
                editor->deleteAdditionalCarets();
            }

            editor->showSuggestionBox = false;
        }

        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_HOME && input){
        if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
        if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);

        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            adeptide->startSelection();

        adeptide->moveCaretBeginningOfLine();
        adeptide->menubar.loseFocus();
        return;
    }

    if(key == GLFW_KEY_END && input){
        if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
        if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);

        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            adeptide->startSelection();

        adeptide->moveCaretEndOfLine();

        adeptide->menubar.loseFocus();
        return;
    }

    if(key == GLFW_KEY_LEFT && input){
        if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
        if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);

        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            adeptide->startSelection();

        #ifdef __APPLE__
        if(mods & GLFW_MOD_CONTROL || mods & GLFW_MOD_SUPER) adeptide->moveCaretBeginningOfWord();
        #else
        if(mods & GLFW_MOD_CONTROL) adeptide->moveCaretBeginningOfWord();
        #endif
        else if(mods & GLFW_MOD_ALT) adeptide->moveCaretBeginningOfSubWord();
        else adeptide->moveCaretLeft();

        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_RIGHT && input){
        if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
        if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);

        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            adeptide->startSelection();

        #ifdef __APPLE__
        if(mods & GLFW_MOD_CONTROL || mods & GLFW_MOD_SUPER) adeptide->moveCaretEndOfWord();
        #else
        if(mods & GLFW_MOD_CONTROL) adeptide->moveCaretEndOfWord();
        #endif
        else if(mods & GLFW_MOD_ALT) adeptide->moveCaretEndOfSubWord();
        else adeptide->moveCaretRight();

        adeptide->menubar.loseFocus();
    }

    if(key == GLFW_KEY_UP && input){
        if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
        if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);

        if(adeptide->terminal && adeptide->terminal->isVisible()){
            adeptide->terminal->up();
        #if _WIN32
        } else if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && CMDCTRL_MOD(mods)){
        #else
        } else if(mods & GLFW_MOD_ALT && CMDCTRL_MOD(mods)){
        #endif
            adeptide->duplicateCaretUp();
        } else if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            adeptide->scrollUp(8);
        } else {
            if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                adeptide->startSelection();

            adeptide->moveCaretUp();

            adeptide->menubar.loseFocus();
        }
    }

    if(key == GLFW_KEY_DOWN && input){
        if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
        if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);

        if(adeptide->terminal && adeptide->terminal->isVisible()){
            adeptide->terminal->down();
        #if _WIN32
        } else if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && CMDCTRL_MOD(mods)){
        #else
        } else if(mods & GLFW_MOD_ALT && CMDCTRL_MOD(mods)){
        #endif
            adeptide->duplicateCaretDown();
        } else if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            adeptide->scrollDown(8);
        } else {
            if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                adeptide->startSelection();

            adeptide->moveCaretDown();

            adeptide->menubar.loseFocus();
        }
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

    if(CMDCTRL_MOD(mods) && input){
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
            if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
            if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);
            adeptide->selectAll();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_L:
            if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
            if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);
            adeptide->selectLine();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_O:
            if(mods & GLFW_MOD_SHIFT) adeptide->openFolder();
            else adeptide->openFile();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_N:
            if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
            if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);
            if(mods & GLFW_MOD_SHIFT) open_playground_menu(adeptide);
            else adeptide->newFile(FileType::ADEPT);
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_I:
            adeptide->updateInsight();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_P:
            adeptide->lookForFile();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_J:
        case GLFW_KEY_G:
            adeptide->gotoLine();
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
            if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
            if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);
            adeptide->cutSelected();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_V:
            adeptide->paste();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_W:
            if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);
            if(adeptide->lineNavigator) adeptide->lineNavigator->setVisibility(false);
            adeptide->removeCurrentEditor();
            adeptide->menubar.loseFocus();
            break;
        case GLFW_KEY_B:
            if(mods & GLFW_MOD_SHIFT) build_and_run_adept_project(adeptide);
            else                      build_adept_project(adeptide);
            break;
        case GLFW_KEY_R:
            adeptide->runFile();
            break;
        case GLFW_KEY_Z:
            if(TextEditor *editor = adeptide->getCurrentEditorAsTextEditor()){
                if(mods & GLFW_MOD_SHIFT) editor->redo();
                else                      editor->undo();
            }
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
    if(adeptide->fileLooker) adeptide->fileLooker->setVisibility(false);

    if(adeptide->menubar.leftClick(xpos, ypos, &adeptide->currentEditorIndex)){
        // Click menu
        adeptide->updateTitle();
    } else if(xpos >= 0.0f && xpos <= 32.0f && ypos >= 24.0f && ypos <= 24.0f + 32.0f){
        // Toggle Explorer Button
        if(adeptide->explorer) adeptide->explorer->toggleVisibility();
    } else if(xpos >= 32.0f && xpos <= 64.0f && ypos >= 24.0f && ypos <= 24.0f + 32.0f){
        // Open Folder Button
        adeptide->openFolder();
    } else if(xpos >= 64.0f && xpos <= 96.0f && ypos >= 24.0f && ypos <= 24.0f + 32.0f){
        // Update Insight Tree Button
        adeptide->updateInsight();
    } else if(xpos >= 96.0f && xpos <= 128.0f && ypos >= 24.0f && ypos <= 24.0f + 32.0f){
        // Update Insight Tree Button
        adeptide->cdFile();
    } else if(adeptide->explorer && adeptide->explorer->leftClick(adeptide, xpos, ypos)){
        // Click explorer
    } else {
        adeptide->menubar.loseFocus();

        TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
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

void new_adept_file(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->newFile(FileType::ADEPT);
    adeptide->menubar.loseFocus();
}

void new_plain_text_file(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->newFile(FileType::PLAIN_TEXT);
    adeptide->menubar.loseFocus();
}

void new_java_file(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->newFile(FileType::JAVA);
    adeptide->menubar.loseFocus();
}

void new_painting_file(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->newFile(FileType::PAINTING);
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

void open_folder_menu(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    adeptide->openFolder();
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
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(editor){
        editor->setFileType(PLAIN_TEXT);
        adeptide->menubar.loseFocus();
    }
}

void language_adept(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(editor){
        editor->setFileType(ADEPT);
        adeptide->menubar.loseFocus();
    }
}

void language_java(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(editor){
        editor->setFileType(JAVA);
        adeptide->menubar.loseFocus();
    }
}

void language_html(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(editor){
        editor->setFileType(HTML);
        adeptide->menubar.loseFocus();
    }
}

void theme_visual_studio(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(editor){
        editor->setSyntaxColorPalette(SyntaxColorPalette::Defaults::VISUAL_STUDIO);
        adeptide->menubar.loseFocus();
    }
}

void theme_fruit_smoothie(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(editor){
        editor->setSyntaxColorPalette(SyntaxColorPalette::Defaults::FRUIT_SMOOTHIE);
        adeptide->menubar.loseFocus();
    }
}

void theme_tropical_ocean(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(editor){
        editor->setSyntaxColorPalette(SyntaxColorPalette::Defaults::TROPICAL_OCEAN);
        adeptide->menubar.loseFocus();
    }
}

void theme_island_campfire(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(editor){
        editor->setSyntaxColorPalette(SyntaxColorPalette::Defaults::ISLAND_CAMPFIRE);
        adeptide->menubar.loseFocus();
    }
}

void theme_one_dark(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(editor){
        editor->setSyntaxColorPalette(SyntaxColorPalette::Defaults::ONE_DARK);
        adeptide->menubar.loseFocus();
    }
}

void about_menu(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE *>(data);
    
    std::string about_string;
    about_string += "VERSION:  AdpetIDE " + std::string(__DATE__) + " " + std::string(__TIME__) + "\n";
    about_string += "INSIGHT:  Adept 2.3 (in development)\n";
    about_string += "PLATFORM: ";

    #if _WIN32
    about_string += "Windows";
    #elif __APPLE__
    about_string += "MacOS";
    #elif __linux__
    about_string += "Linux";
    #else
    about_string += "Unix";
    #endif

    adeptide->createMessage(about_string, 4.0);
    adeptide->menubar.loseFocus();
}

void maximize(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE *>(data);
    glfwMaximizeWindow(adeptide->window);
    adeptide->menubar.loseFocus();
}


void selection_menu(void *data){
    DropdownMenu *dropdownMenu = static_cast<DropdownMenu*>(data);
    dropdownMenu->isOpen = true;
}

void build_menu(void *data){
    DropdownMenu *dropdownMenu = static_cast<DropdownMenu*>(data);
    dropdownMenu->isOpen = true;
}

void help_menu(void *data){
    DropdownMenu *dropdownMenu = static_cast<DropdownMenu*>(data);
    dropdownMenu->isOpen = true;
}

void build_adept_project(void *data){
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(data);
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(editor){
        if(editor->filename == ""){
            adeptide->createMessage("File must be saved in order to build", 2.0);
        }
        else if (system((adeptide->settings.adept_compiler + " \"" + editor->filename + "\" > \"" + adeptide->root + "adept.log\" 2>&1").c_str()))
        {
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
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(editor){
        if(editor->filename == ""){
            adeptide->createMessage("File must be saved in order to build", 2.0);
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

double distance(double x1, double y1, double x2, double y2){
    double xdiff = x1 - x2;
    double ydiff = y1 - y2;
    return sqrt(xdiff * xdiff + ydiff * ydiff);
}
