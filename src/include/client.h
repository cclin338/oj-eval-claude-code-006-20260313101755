#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <queue>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// Game state tracking
std::vector<std::vector<char>> map_state;  // Current state of the map
std::vector<std::vector<bool>> is_safe;    // Cells known to be safe
std::vector<std::vector<bool>> is_mine_cell; // Cells known to be mines
int marked_mines = 0;
int visited_cells = 0;

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize all global variables
  map_state.assign(rows, std::vector<char>(columns, '?'));
  is_safe.assign(rows, std::vector<bool>(columns, false));
  is_mine_cell.assign(rows, std::vector<bool>(columns, false));
  marked_mines = 0;
  visited_cells = 0;

  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  for (int i = 0; i < rows; i++) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < columns; j++) {
      map_state[i][j] = line[j];
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
// Helper function to get adjacent cells
std::vector<std::pair<int, int>> getAdjacent(int r, int c) {
  std::vector<std::pair<int, int>> adj;
  int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
  for (int k = 0; k < 8; k++) {
    int nr = r + dx[k];
    int nc = c + dy[k];
    if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
      adj.push_back({nr, nc});
    }
  }
  return adj;
}

// Analyze the map and mark safe/mine cells
void analyzeMap() {
  bool changed = true;
  while (changed) {
    changed = false;
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < columns; c++) {
        // Only analyze visited numbered cells
        if (map_state[r][c] >= '0' && map_state[r][c] <= '8') {
          int num = map_state[r][c] - '0';
          auto adj = getAdjacent(r, c);

          int unknown_count = 0;
          int marked_count = 0;
          std::vector<std::pair<int, int>> unknown_cells;

          for (auto [nr, nc] : adj) {
            if (map_state[nr][nc] == '?' || map_state[nr][nc] == '@') {
              if (map_state[nr][nc] == '@' || is_mine_cell[nr][nc]) {
                marked_count++;
              } else if (!is_safe[nr][nc]) {
                unknown_count++;
                unknown_cells.push_back({nr, nc});
              }
            }
          }

          // If all mines around are found, remaining cells are safe
          if (marked_count == num && unknown_count > 0) {
            for (auto [nr, nc] : unknown_cells) {
              if (!is_safe[nr][nc]) {
                is_safe[nr][nc] = true;
                changed = true;
              }
            }
          }

          // If number of unknown + marked equals mine count, all unknowns are mines
          if (marked_count + unknown_count == num && unknown_count > 0) {
            for (auto [nr, nc] : unknown_cells) {
              if (!is_mine_cell[nr][nc]) {
                is_mine_cell[nr][nc] = true;
                changed = true;
              }
            }
          }
        }
      }
    }
  }
}

void Decide() {
  // Analyze the current map state
  analyzeMap();

  // Priority 1: Visit safe cells
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < columns; c++) {
      if (is_safe[r][c] && map_state[r][c] == '?') {
        Execute(r, c, 0);
        return;
      }
    }
  }

  // Priority 2: Mark known mines
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < columns; c++) {
      if (is_mine_cell[r][c] && map_state[r][c] == '?') {
        Execute(r, c, 1);
        return;
      }
    }
  }

  // Priority 3: Use auto-explore on cells where all mines are marked
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < columns; c++) {
      if (map_state[r][c] >= '0' && map_state[r][c] <= '8') {
        int num = map_state[r][c] - '0';
        auto adj = getAdjacent(r, c);

        int marked_count = 0;
        bool has_unknown = false;

        for (auto [nr, nc] : adj) {
          if (map_state[nr][nc] == '@') {
            marked_count++;
          } else if (map_state[nr][nc] == '?') {
            has_unknown = true;
          }
        }

        if (marked_count == num && has_unknown) {
          Execute(r, c, 2);
          return;
        }
      }
    }
  }

  // Priority 4: If no safe move found, take a calculated risk
  // Find the cell with the lowest probability of being a mine
  double best_prob = 2.0;
  int best_r = -1, best_c = -1;

  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < columns; c++) {
      if (map_state[r][c] == '?' && !is_mine_cell[r][c]) {
        // Calculate probability based on adjacent cells
        auto adj = getAdjacent(r, c);
        double prob = 0.0;
        int count = 0;

        for (auto [nr, nc] : adj) {
          if (map_state[nr][nc] >= '0' && map_state[nr][nc] <= '8') {
            int num = map_state[nr][nc] - '0';
            int unknown = 0;
            int marked = 0;

            for (auto [nnr, nnc] : getAdjacent(nr, nc)) {
              if (map_state[nnr][nnc] == '?') {
                if (is_mine_cell[nnr][nnc]) {
                  marked++;
                } else {
                  unknown++;
                }
              } else if (map_state[nnr][nnc] == '@') {
                marked++;
              }
            }

            if (unknown > 0) {
              prob += (double)(num - marked) / unknown;
              count++;
            }
          }
        }

        if (count > 0) {
          prob /= count;
        } else {
          // No information, use global probability
          int total_unknown = 0;
          for (int i = 0; i < rows; i++) {
            for (int j = 0; j < columns; j++) {
              if (map_state[i][j] == '?' && !is_mine_cell[i][j]) {
                total_unknown++;
              }
            }
          }
          int unmarked_mines = total_mines - marked_mines;
          prob = (double)unmarked_mines / total_unknown;
        }

        if (prob < best_prob) {
          best_prob = prob;
          best_r = r;
          best_c = c;
        }
      }
    }
  }

  if (best_r != -1) {
    Execute(best_r, best_c, 0);
    return;
  }

  // Fallback: just visit first unknown cell
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < columns; c++) {
      if (map_state[r][c] == '?') {
        Execute(r, c, 0);
        return;
      }
    }
  }
}

#endif