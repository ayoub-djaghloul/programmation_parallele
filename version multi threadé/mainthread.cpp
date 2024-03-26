#include <iostream>
#include <fstream>
#include <vector>
#include <thread>       // Pour std::thread
#include <mutex>        // Pour std::mutex
#include <atomic>       // Pour std::atomic_bool


int ROWS;
int COLS;
clock_t start, end;
enum TileColor {
    EMPTY,
    BLUE,
    GREEN,
    RED
};

class MacMahonTile {
public:
    MacMahonTile() : colors{EMPTY, EMPTY, EMPTY, EMPTY} {}

    void setColors(TileColor top, TileColor right, TileColor bottom, TileColor left) {
        colors[0] = top;
        colors[1] = right;
        colors[2] = bottom;
        colors[3] = left;
    }

    TileColor getColorAt(int position) const {
        return colors[position];
    }

private:
    TileColor colors[4];
};

class MacMahonBoard {
public:
    MacMahonBoard(int rows, int cols) : ROWS(rows), COLS(cols), board(rows, std::vector<MacMahonTile>(cols)) {}

    void placeTile(int row, int col, const MacMahonTile &tile) {
        board[row][col] = tile;
    }

    const MacMahonTile& getTileAt(int row, int col) const {
        return board[row][col];
    }

    void display() const {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                for (int k = 0; k < 4; ++k) {
                    switch (board[i][j].getColorAt(k)) {
                        case RED:
                            std::cout << "R";
                            break;
                        case GREEN:
                            std::cout << "G";
                            break;
                        case BLUE:
                            std::cout << "B";
                            break;
                        default:
                            std::cout << " ";
                            break;
                    }
                }
                std::cout << "\t";
            }
            std::cout << std::endl;
        }
    }

private:
    const int ROWS, COLS;
    std::vector<std::vector<MacMahonTile>> board;
};

bool canPlaceTile(MacMahonBoard& board, int row, int col, const MacMahonTile& tile) {
    // Vérifiez le côté supérieur
    if(row > 0 && board.getTileAt(row - 1, col).getColorAt(3) != tile.getColorAt(1)) {
        return false;
    }
    // Vérifiez le côté gauche
    if(col > 0 && board.getTileAt(row, col - 1).getColorAt(2) != tile.getColorAt(0)) {
        return false;
    }
    // Vérifiez le côté gauche du panneau supérieur
    if(col > 0 && row ==0 && tile.getColorAt(1) != board.getTileAt(row, col - 1).getColorAt(1)) {
        return false;
    }
    // Vérifiez le côté supérieur du tableau de gauche
    if(row > 0 && col ==0 && tile.getColorAt(0) != board.getTileAt(row - 1, col).getColorAt(0)) {
        return false;
    }
    // Vérifiez le côté droit du panneau supérieur
    if(col == COLS - 1 && row > 0 && tile.getColorAt(2) != board.getTileAt(row - 1, col).getColorAt(2)) {
        return false;
    }
    // Vérifiez le côté gauche du panneau inférieur
    if(col > 0 && row == ROWS - 1 && tile.getColorAt(3) != board.getTileAt(row, col - 1).getColorAt(3)) {
        return false;
    }
    //vérifiez le côté gauche et supérieur de la première tuile
    if (row == 0 && col == 0 && tile.getColorAt(0) != tile.getColorAt(1)) {
        return false;
    }

    return true;
}



TileColor charToTileColor(char c) {
    switch (c) {
        case 'R':
            return RED;
        case 'G':
            return GREEN;
        case 'B':
            return BLUE;
        default:
            return EMPTY;
    }
}

void readDataFromFile(const std::string& filename, std::vector<MacMahonTile>& tiles) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Impossible d'ouvrir le fichier." << std::endl;
        return;
    }

    file >> ROWS >> COLS;
    tiles.clear(); // Efface les tuiles avant de les remplir à nouveau.

    for (int i = 0; i <= ROWS * COLS; ++i) {
        char top, right, bottom, left;
        file >> left >> top >> right >> bottom;
        MacMahonTile tile;
        tile.setColors(charToTileColor(left), charToTileColor(top), charToTileColor(right), charToTileColor(bottom));
        tiles.push_back(tile);
    }

    file.close();
}

std::atomic_bool solutionFound(false);  // This will be set to true if a solution is found
std::mutex mtx;                         // Mutex to synchronize access to shared resources

bool solve(MacMahonBoard& board, int row, int col, std::vector<MacMahonTile>& tiles) {
    // Vérifiez si une solution a déjà été trouvée
    if (solutionFound.load()) {
        return false;
    }
    // Si nous arrivons à la fin du tableau, nous avons terminé
    if(row == ROWS) {
        // Nous avons atteint la fin du tableau
        return true; // Toutes les tuiles ont été placées avec succès
    }



    for(int i = 0; i < tiles.size(); i++) {
        if(canPlaceTile(board, row, col, tiles[i])) {
            board.placeTile(row, col, tiles[i]);

            // Supprime la tuile des tuiles disponibles
            MacMahonTile usedTile = tiles[i];
            tiles.erase(tiles.begin() + i);
            int newRow = (col == COLS - 1 )? row + 1 : row;
            int newCol =( col == COLS - 1 )? 0 : col + 1;

            // Essayez de manière récursive de résoudre la position suivante
            if(solve(board, newRow, newCol, tiles)) {
                return true;
            }

            // Si nous sommes arrivés ici, le placement était erroné. annuler.
            tiles.insert(tiles.begin() + i, usedTile);
            board.placeTile(row, col, MacMahonTile());  // Ajoutez cette ligne pour supprimer la tuile placée.
        }
    }
    return false; // Impossible de placer une tuile à la position actuelle
}



void solveWithThread(MacMahonBoard initialBoard, std::vector<MacMahonTile> initialTiles, int startingTileIndex) {
    if (solutionFound.load()) {
        return; // Sortez plus tôt si une solution a déjà été trouvée
    }
    // Utilisez une copie locale du plateau et des tuiles pour chaque thread
    MacMahonBoard board = initialBoard;
    std::vector<MacMahonTile> tiles = initialTiles;      // Copie locale des tuiles
    // Placez la tuile initiale sur le plateau
    board.placeTile(0, 0, tiles[startingTileIndex]);
    tiles.erase(tiles.begin() + startingTileIndex);

    // Exécutez la fonction séquentielle ici
    if (solve(board, 0, 0, tiles) && !solutionFound.exchange(true)) {
        std::lock_guard<std::mutex> lock(mtx);
        board.display();
    }
}
int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    std::vector<MacMahonTile> tiles;
    readDataFromFile(filename, tiles);

    if(tiles.size()-1 != ROWS * COLS) {
        std::cout << "Le nombre de tuiles ne correspond pas à la taille du plateau spécifiée." << std::endl;
        std::cout << "Nombre de tuiles: " << tiles.size() << std::endl;
        std::cout << "Taille du plateau: " << ROWS << "x" << COLS << std::endl;
        return 1;
    }

    MacMahonBoard board(ROWS, COLS);

    auto start = std::chrono::high_resolution_clock::now();

    // Créez des fils de discussion pour chaque position de tuile de départ
    std::vector<std::thread> threads;
    for (int i = 0; i < tiles.size()-1; i++) {
    std::thread t(solveWithThread, board, tiles, i);
    threads.push_back(std::move(t));    
    }

    // Attendez que tous les sujets soient terminés
    for (std::thread &t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    if (solutionFound) {
        std::cout << "Solution found." << std::endl;
        std::cout << "Temps pris : " << elapsed.count() << " secondes" << std::endl;
        std::cout << "Le nombre de threads: " << threads.size() << std::endl;
    } else {
        std::cout << "No solution found." << std::endl;
    }
    return 0;
}
