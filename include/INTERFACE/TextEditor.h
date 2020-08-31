
#ifndef TEXT_EDITOR_H_INCLUDED
#define TEXT_EDITOR_H_INCLUDED

#include <string>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>

#include "OPENGL/Shader.h"
#include "INTERFACE/Font.h"
#include "INTERFACE/Caret.h"
#include "INTERFACE/Selection.h"
#include "INTERFACE/FileType.h"
#include "INTERFACE/RichText.h"
#include "INTERFACE/Settings.h"
#include "INTERFACE/SymbolWeight.h"
#include "INTERFACE/GenericEditor.h"
#include "INTERFACE/SuggestionBox.h"
#include "UTIL/ChangeRecord.h"

#include "INSIGHT/insight.h"

typedef char InsightCreationResult;
#define InsightCreationResultNothingNew 0
#define InsightCreationResultSuccess 1
#define InsightCreationResultFailure 2
#define InsightCreationResultNotAdept 3
#define InsightCreationResultSilentSuccess 4

class TextEditor : public GenericEditor {
    SyntaxColorPalette palette;
    
    std::string lineNumbersText;
    TextModel lineNumbersModel;
    bool hasLineNumbersModel;
    bool lineNumbersUpdated;

    Matrix4f transformationMatrix;

    Caret mainCaret;
    Selection *selection;
    Caret selectionStartCaret;
    std::vector<Caret*> additionalCarets;
    std::vector<Selection*> additionalSelections;
    std::vector<Caret*> additionalSelectionStartCarets;
    bool selecting;

    int scroll;
    float textXOffset;

    // Fields in this grouping are controled by the mutex 'insightMutex'
    std::thread insightThread;
    std::atomic<bool> insightRunning;
    bool hasCompiler;
    compiler_t compiler;
    double lastPassiveInsightUpdate;

    RichText richText;
    SuggestionBox suggestionBox;

    ChangeRecord changeRecord;

    void handleSelection();
    float calculateScrollOffset();
    float calculateScrollOffset(size_t line);
    void generateLineNumbersText();
    void generateSuggestions();

public:
    bool showSuggestionBox;
    
    void makeInsight(bool storeCreationResult = false, bool fromMemory = false, bool showSuccessMessage = true);

    // Fields in this grouping are controled by the mutex 'insightMutex'
    std::mutex insightMutex;
    InsightCreationResult insightCreationResult;

    ~TextEditor();
    void load(Settings *settings, Font *font, Texture *fontTexture, float maxWidth, float maxHeight);
    void render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets);
    TextModel *getFilenameModel();
    void updateFilenameModel();
    size_t getDisplayFilenameLength();
    FileType getFileType();
    void setFileType(const FileType &type);
    void setSyntaxColorPalette(const SyntaxColorPalette &palette);
    void setSyntaxColorPalette(const SyntaxColorPalette::Defaults &presetPalette);
    void resize(float width, float height);
    void snapCaretToPosition(float x, float y);

    void type(const std::string &characters);
    void type(char character);
    void typeBlock();
    void typeExpression();
    void typeArrayAccess();
    void typeString();
    void typeCString();
    void backspace();
    void backspaceForCaret(Caret *caret);
    void del();
    void smartBackspace();
    void smartDel();
    bool smartRemove();
    void backspaceLine();
    void delLine();
    void startSelection(bool snapSelectionCaret = true);
    void endSelection();
    void destroySelection();
    void deleteSelected();
    bool hasSelection();
    void deleteRange(size_t beginning, size_t end);
    void copySelected(GLFWwindow *window);
    void cutSelected(GLFWwindow *window);
    void paste(GLFWwindow *window);
    void tab();
    void nextLine();
    void nextPrecedingLine();
    void finishSuggestion();
    void maybeUpdatePassiveInsight();

    void moveCaretToPosition(size_t position);
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
    void adjustViewForCaret();
    void focusViewForCaret();
    void selectAll();
    void selectLine();
    void relationallyIncreaseCaret(Caret *targetCaret, size_t amount);
    void relationallyDecreaseCaret(Caret *targetCaret, size_t amount);
    void relationallyMaintainIncrease(Caret *targetCaret, size_t amount);
    void relationallyMaintainDecrease(Caret *targetCaret, size_t amount);
    void gotoLine(int lineNumber);
    void forEachCaret(std::function<void (Caret *)> lambda_function);

    void duplicateCaretUp();
    void duplicateCaretDown();
    void deleteAdditionalCarets();

    void setOffset(float xOffset, float yOffset);
    float getNetXOffset();
    float getNetYOffset();

    void loadTextFromFile(const std::string &filename);
    void saveFile();

    void undo();
    void redo();
    void undoChange(Change *change);
    void redoChange(Change *change);

    void getRowAndColumnAt(double xpos, double ypos, int *out_row, int *out_column);
    size_t getCaretPosition();

    compiler_t *borrowCompiler();
    void returnCompiler();

    const std::string &getText();

    TextEditor *asTextEditor();
    ImageEditor *asImageEditor();
};

#endif // TEXT_EDITOR_H_INCLUDED