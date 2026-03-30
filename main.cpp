#include "classes/library.h"
#include "menu.h"
#include "save_handler.h"

int main() {
    Library library;
    SaveHandler::loadAll(library);
    runMenu(library);
    return 0;
}
