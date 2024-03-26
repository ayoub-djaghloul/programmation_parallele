#include <iostream>
#include <fstream>
#include <vector>
#include <thread>       // For std::thread
#include <mutex>        // For std::mutex
#include <atomic>       // For std::atomic_bool
#include <condition_variable>
#include <queue>
#include <functional>
#include <future>


class ThreadPool {
public:
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i)
            workers.emplace_back(
                [this] {
                    for(;;) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->condition.wait(lock,
                                [this] { return this->stop || !this->tasks.empty(); });
                            if(this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                }
            );
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );        
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if(stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([task](){ (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker: workers)
            worker.join();
    }

private:
    std::vector< std::thread > workers;
    std::queue< std::function<void()> > tasks;
    
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};
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
// Main fonction
int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];

    std::vector<MacMahonTile> tiles;
    readDataFromFile(filename, tiles);

    MacMahonBoard board(ROWS, COLS);

    auto start = std::chrono::high_resolution_clock::now();

    ThreadPool pool(std::thread::hardware_concurrency()); // Crée un pool avec le nombre de threads supportés par le matériel


    std::vector<std::future<bool>> futures;
    for (int i = 0; i < tiles.size()-1; i++) {
        futures.emplace_back(pool.enqueue([=] {
            solveWithThread(board, tiles, i);
            return true;
        }));
    }
    // Ici, le pool sera automatiquement détruit à la fin de la portée et joindra tous les threads
    // attendez les futures si vous avez besoin du résultat
    for (auto &f : futures) {
        f.wait();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    if (solutionFound) {
        std::cout << "Solution found." << std::endl;
        std::cout << "Time taken: " << elapsed.count() << " seconds" << std::endl;
    } else {
        std::cout << "No solution found." << std::endl;
    }
    return 0;
}
