#include <iostream>
#include <fstream>
#include <vector>

int ROWS;
int COLS;

enum TileColor {
    EMPTY,
    BLUE,
    GREEN,
    RED
};

class MacMahonTile {
public:
    MacMahonTile() {
        colors.resize(4, EMPTY);
    }

    void setColors(TileColor top, TileColor right, TileColor bottom, TileColor left) {
        colors[0] = top;
        colors[1] = right;
        colors[2] = bottom;
        colors[3] = left;
    }

    TileColor getColorAt(int position) const {
        if (position >= 0 && position < 4) {
            return colors[position];
        }
        return RED;
    }

private:
    std::vector<TileColor> colors;
};

class MacMahonBoard {
public:
    MacMahonBoard() : board(ROWS, std::vector<MacMahonTile>(COLS)) {}

    void placeTile(int row, int col, const MacMahonTile &tile) {
        if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
            board[row][col] = tile;
        }
    }

    const MacMahonTile& getTileAt(int row, int col) const {
        return board[row][col];
    }

    void display() const {
         for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            // Affichez toutes les couleurs de la tuile (les 4 côtés).
            for (int k = 0; k < 4; ++k) {
                //affichage des couleurs de la tuile comme demandé lettre pas chiffre 
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

bool solve(MacMahonBoard& board, int row, int col, std::vector<MacMahonTile>& tiles) {

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


int main(int argc, char* argv[]) {
    if(argc != 2) { // On s'assure d'avoir le bon nombre d'arguments : le nom du programme, le nom du fichier, ROWS et COLS
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1; // quitte avec une erreur
    }

    std::string filename = argv[1];

    std::vector<MacMahonTile> tiles;

    readDataFromFile(filename, tiles);
    MacMahonBoard board;


    //calcul du temps
    clock_t start, end;
    start = clock();
    if (solve(board, 0, 0, tiles)) {
        std::cout << "Solution found:" << std::endl;
        board.display();
    } else {
        std::cout << "No solution found." << std::endl;
    }
    //calcul de fin de temps
    end = clock();
    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
    std::cout << "Time taken by program is : " << std::fixed << time_taken << " sec " << std::endl;

    return 0;
}