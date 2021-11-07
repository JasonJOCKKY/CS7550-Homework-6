#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>
#include <ctime>
#include <chrono>

using namespace std;
using std::chrono::high_resolution_clock;
using std::chrono::duration;

// Sudoku CSP problem.
// Variables: Cells in the board.
// Domian: D_i = {1,2,3,4,5,6,7,8,9}
// Constraints: There are no same number in each row, col, and the region.
class SudokuSolver {
    public:
        // domain[i][j][0] represents the domain size.
        // domain[i][j][k] represents whether the number k is in domain or not.
        int domain[9][9][10]; 
        vector<pair<int, int>> res_selected_variable;
        vector<int> res_domain_size;
        vector<int> res_degree_selected_variable;
        vector<int> res_value_assign;
        vector<vector<vector<char>>> res_board;

        bool solve_sudoku(vector<vector<char>>& board) {
            initial_board(board);
            return backtracking_sudoku(board);
        }

        bool backtracking_sudoku(vector<vector<char>>& board)
        {
            // Select a variable according to MRV.
            // The Degree Heuristic would be used for tie-breaker among MRV variables.
            int degree_selected_variable;
            pair<int, int> cell = minimum_remaining_values(board, degree_selected_variable);
            if (cell.first == -1 && cell.second == -1) return true;
            if (domain[cell.first][cell.second][0] == 0) return false;

            // Save the current domain.
            int temp_domain[9][9][10];
            memcpy(temp_domain, domain, sizeof(domain));

            // iterate possible numbers.
            for (int num = 1; num <= 9; num++)
                if (domain[cell.first][cell.second][num] == 1)
                {
                    // Set the number to the board.
                    int domain_size = domain[cell.first][cell.second][0];
                    assign_value(cell.first, cell.second, num, board);

                    // forward checking.
                    // backtracking search for the results.
                    if ( forward_checking(board) ) {
                        record_results(cell, board, domain_size, degree_selected_variable, num);
                        if (backtracking_sudoku(board) == true) return true;
                    }
                    // else {
                    //     cout<<"LOG: forward_checking - False"<<endl;
                    // }

                    // Reset the domain.
                    memcpy(domain, temp_domain, sizeof(domain));
                }

            // there is no solution for current board state.
            // board[cell.first][cell.second] = '.';
            return false;
        }

        // initializing the domain according to the board.
        void initial_board(vector<vector<char>>& board)
        {
            for (int i = 0; i < 9; i++)
                for (int j = 0; j < 9; j++)
                {
                    domain[i][j][0] = 9;
                    for (int k = 1; k <= 9; k++)
                        domain[i][j][k] = 1;
                }
            for (int i = 0; i < 9; i++)
                for (int j = 0; j < 9; j++)
                    if (board[i][j] != '.')
                        assign_value(i, j, board[i][j]-'0', board);
        }

        // Set the number to board and update the domain.
        void assign_value(int i, int j, int num, vector<vector<char>>& board)
        {
            board[i][j] = num+'0';
            int ii = (i/3) * 3, jj = (j/3) * 3;
            for (int k = 0; k < 9; k++)
            {   
                // domain for column and row.
                domain[i][k][0] -= domain[i][k][num];
                domain[i][k][num] = 0;
                domain[k][j][0] -= domain[k][j][num];
                domain[k][j][num] = 0;

                // domain for the region.
                domain[ii+(k/3)][jj+(k%3)][0] -= domain[ii+(k/3)][jj+(k%3)][num];
                domain[ii+(k/3)][jj+(k%3)][num] = 0;
            }
            
        }
        
        // Select a variable based on Minimum remaining values (MRV).
        // The Degree Heuristic would be used for tie-breaker among MRV variables.
        pair<int, int> minimum_remaining_values(vector<vector<char>>& board, int &degree_selected_variable) {
            int row = -1, col = -1, choice = 10;
            int degree = -1;
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    if (board[i][j] == '.' ) {
                        if (domain[i][j][0] < choice) {
                            row = i;
                            col = j;
                            choice = domain[i][j][0];
                            degree = degree_heuristic(board, i, j);
                        }
                        else if(domain[i][j][0] == choice) {
                            // Tie-breaker among MRV variables by Degree Heuristic.
                            int cur_degree = degree_heuristic(board, i, j);
                            if (degree < cur_degree) {
                                degree = cur_degree;
                                row = i;
                                col = j;
                                choice = domain[i][j][0];
                            }
                        }
                    }
                } 
            }
            degree_selected_variable = degree;
            return {row, col};
        }

        // The Degree Heuristic would be used for tie-breaker among MRV variables.
        int degree_heuristic(vector<vector<char>>& board, int row, int col) {
            int degree = 0;

            // Count empty cell in row and column given variable.
            for (int i = 0; i < 9; i++) {
                if(board[row][i] == '.' && i!=col)
                    degree++;
                if(board[i][col] == '.' && i!=row)
                    degree++;
            }

            // Count empty cell in region given variable.
            int ii = (row/3) * 3, jj = (col/3) * 3;
            for (int k = 0; k < 9; k++) {
                int i = ii+(k/3);
                int j = jj+(k%3);
                if(i!=row && j!=col && board[i][j]=='.') {
                    degree++;
                }
            }
            return degree;
        }

        // Keep track of remaining legal values for unassigned variables.
        // When any variable has no legal values return false.
        bool forward_checking(vector<vector<char>>& board) {
            for (int i = 0; i < 9; i++)
                for (int j = 0; j < 9; j++)
                {
                    if (board[i][j] == '.' && domain[i][j][0]==0) {
                        return false;
                    }
                }
            return true;
        }
        
        //  reocrd results for the report.
        void record_results(pair<int, int> cell, vector<vector<char>> board, int domain_size, int degree_selected_variable, int value_assign) {
            res_selected_variable.push_back(cell);
            res_board.push_back(board); 
            res_domain_size.push_back( domain_size );
            res_degree_selected_variable.push_back(degree_selected_variable);
            res_value_assign.push_back(value_assign);
        }

        // print results.
        void print_record_results() {
            for (int i=0; i<5; i++) {
                cout<<"Step "<<i+1<<endl;

                cout<<"Variable selected (row,column):"<<endl;
                cout<<"("<<res_selected_variable[i].first+1<<","<<res_selected_variable[i].second+1<<")"<<endl;
                cout<<endl;

                cout<<"The domain size of the selected variable:"<<endl;
                cout<<res_domain_size[i]<<endl;
                cout<<endl;

                cout<<"The degree of the selected variable:"<<endl;
                cout<<res_degree_selected_variable[i]<<endl;
                cout<<endl;

                cout<<"The value assigned to the selected variable:"<<endl;
                cout<<res_value_assign[i]<<endl;
                cout<<endl;

                cout<<"Current board state:"<<endl;
                print_board(res_board[i]);
                cout<<endl;
            }
        }

        // print board.
        void print_board(vector<vector<char>>& board) {
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++)
                {
                   cout<<board[i][j]<<" ";
                }
                cout<<endl;
            }
            cout<<endl;
        }
};


int main(){

    vector<vector<char> > board_case_1 {
        {'.','.','1',  '.','.','2',  '.','.','.'},
        {'.','.','5',  '.','.','6',  '.','3','.'},
        {'4','6','.',  '.','.','5',  '.','.','.'},

        {'.','.','.',  '1','.','4',  '.','.','.'},
        {'6','.','.',  '8','.','.',  '1','4','3'},
        {'.','.','.',  '.','9','.',  '5','.','8'},

        {'8','.','.',  '.','4','9',  '.','5','.'},
        {'1','.','.',  '3','2','.',  '.','.','.'},
        {'.','.','9',  '.','.','.',  '3','.','.'}
    };

    vector<vector<char> > board_case_2 {
        {'.','.','5',  '.','1','.',  '.','.','.'},
        {'.','.','2',  '.','.','4',  '.','3','.'},
        {'1','.','9',  '.','.','.',  '2','.','6'},

        {'2','.','.',  '.','3','.',  '.','.','.'},
        {'.','4','.',  '.','.','.',  '7','.','.'},
        {'5','.','.',  '.','.','7',  '.','.','1'},

        {'.','.','.',  '6','.','3',  '.','.','.'},
        {'.','6','.',  '1','.','.',  '.','.','.'},
        {'.','.','.',  '.','7','.',  '.','5','.'},
    };

    vector<vector<char> > board_case_3 {
        {'6','7','.',  '.','.','.',  '.','.','.'},
        {'.','2','5',  '.','.','.',  '.','.','.'},
        {'.','9','.',  '5','6','.',  '2','.','.'},

        {'3','.','.',  '.','8','.',  '9','.','.'},
        {'.','.','.',  '.','.','.',  '8','.','1'},
        {'.','.','.',  '4','7','.',  '.','.','.'},

        {'.','.','8',  '6','.','.',  '.','9','.'},
        {'.','.','.',  '.','.','.',  '.','1','.'},
        {'1','.','6',  '.','5','.',  '.','7','.'},
    };

    vector<vector<vector<char> >> board_cases;
    board_cases.push_back(board_case_1);
    board_cases.push_back(board_case_2);
    board_cases.push_back(board_case_3);
    
    

    for (int i=0; i<board_cases.size(); i++) {
        
        SudokuSolver solver = SudokuSolver();

        auto start_time = high_resolution_clock::now();
        vector<vector<char> > board = board_cases[i];
        bool res = solver.solve_sudoku(board);
        auto end_time = high_resolution_clock::now();
        duration<double, std::milli> ms_double = end_time - start_time;

        cout<<"============================================================"<<endl;
        cout<<"CASE #"<<i+1<<endl;
        cout<<"============================================================"<<endl;
        solver.print_record_results();

        cout<<"Final solution:"<<endl;
        solver.print_board(board);

        cout<<"CPU execution time in seconds: " << ms_double.count()/1000.0 << "s"<<endl;
        cout<<endl;
    }


    return 0;
}