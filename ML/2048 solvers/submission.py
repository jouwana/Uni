import copy

import logic
import random
from AbstractPlayers import *
import time

# commands to use for move players. dictionary : Move(enum) -> function(board),
# all the functions {up,down,left,right) receive board as parameter and return tuple of (new_board, done, score).
# new_board is according to the step taken, done is true if the step is legal, score is the sum of all numbers that
# combined in this step.
# (you can see GreedyMovePlayer implementation for example)
commands = {Move.UP: logic.up, Move.DOWN: logic.down,
            Move.LEFT: logic.left, Move.RIGHT: logic.right}


# generate value between {2,4} with probability p for 4
def gen_value(p=PROBABILITY):
    return logic.gen_two_or_four(p)


# the hueristic function we will use (the same from greedy)
def getHueristicVal(board, start, time_limit):
    optional_moves_score = {}
    stop = False
    for move in Move:
        elapsed = time.time()
        if elapsed - start >= time_limit - 0.05:
            break
        new_board, done, score = commands[move](board)
        empty = 0
        edges = 0
        pairs = 0
        if done:
            for a in range(4):
                if stop: break
                for b in range(4):
                    if stop: break
                    if new_board[a][b] == 0:
                        empty += 1
                    else:
                        if a != 0: pairs += 2 * new_board[a][b] if new_board[a][b] == new_board[a - 1][b] else 0
                        if a != 3: pairs += 2 * new_board[a][b] if new_board[a][b] == new_board[a + 1][b] else 0
                        if b != 0: pairs += 2 * new_board[a][b] if new_board[a][b] == new_board[a][b - 1] else 0
                        if b != 3: pairs += 2 * new_board[a][b] if new_board[a][b] == new_board[a][b + 1] else 0
                    elapsed = time.time()
                    if elapsed - start >= time_limit - 0.05:
                        stop = True
                        break
            for a in range(4):
                edges = new_board[0][a] + new_board[3][a] + new_board[a][0] + new_board[a][3]
                elapsed = time.time()
                if elapsed - start >= time_limit - 0.05:
                    stop = True
                    break
            optional_moves_score[move] = 100 * empty + edges + pairs + 0.1 * score
    if len(optional_moves_score) == 0:
        return Move.UP, -1
    move = max(optional_moves_score, key=optional_moves_score.get)
    return move, optional_moves_score[move]


class GreedyMovePlayer(AbstractMovePlayer):
    """Greedy move player provided to you (no need to change),
    the player receives time limit for a single step and the board as parameter and return the next move that gives
    the best score by looking one step ahead.
    """

    def get_move(self, board, time_limit) -> Move:
        optional_moves_score = {}
        for move in Move:
            new_board, done, score = commands[move](board)
            if done:
                optional_moves_score[move] = score

        return max(optional_moves_score, key=optional_moves_score.get)


class RandomIndexPlayer(AbstractIndexPlayer):
    """Random index player provided to you (no need to change),
    the player receives time limit for a single step and the board as parameter and return the next indices to
    put 2 randomly.
    """

    def get_indices(self, board, value, time_limit) -> (int, int):
        a = random.randint(0, len(board) - 1)
        b = random.randint(0, len(board) - 1)
        while board[a][b] != 0:
            a = random.randint(0, len(board) - 1)
            b = random.randint(0, len(board) - 1)
        return a, b


# part A
class ImprovedGreedyMovePlayer(AbstractMovePlayer):
    """Improved greedy Move Player,
    implement get_move function with greedy move that looks only one step ahead with heuristic.
    (you can add helper functions as you want).
    """

    def __init__(self):
        AbstractMovePlayer.__init__(self)

    def get_move(self, board, time_limit) -> Move:
        start = time.time()
        move, val = getHueristicVal(board, start, time_limit)
        return move


# part B
class MiniMaxMovePlayer(AbstractMovePlayer):
    """MiniMax Move Player,
    implement get_move function according to MiniMax algorithm
    (you can add helper functions as you want).
    """

    def __init__(self):
        AbstractMovePlayer.__init__(self)
        # TODO: add here if needed

    def get_move(self, board, time_limit) -> Move:
        start = time.time()
        res_move = Move.UP  # the base move
        max_score = float('-inf')
        depth = 1
        # iterative deepening
        while time.time() - start < time_limit - 0.1:
            move, score = self.search_agent(board, "PLAYER", start, time_limit, 1, depth)
            if score > max_score:
                max_score = score
                res_move = move
            depth += 1
        return res_move

    # TODO: add here helper functions in class, if needed
    def search_agent(self, board, agent, start, time_limit, depth, max_depth):
        chosen_move = None
        # evaluate on final depth
        if depth > max_depth or max_depth == 0:
            move, val = getHueristicVal(board, start, time_limit)
            return val
        if time.time() - start >= time_limit - 0.1:
            return float('-inf')

        # player turn
        if agent == "PLAYER":
            currMax = float('-inf')
            for move in Move:
                new_board, done, score = commands[move](board)
                if done:
                    # switch player
                    v = self.search_agent(new_board, "RIVAL", start, time_limit, depth + 1, max_depth)
                    if v > currMax and depth == 1:
                        chosen_move = move
                    currMax = max(currMax, v)
            if depth == 1:
                return chosen_move, currMax
            return currMax

        # index turn
        else:
            v = float('inf')
            for a in range(4):
                for b in range(4):
                    if board[a][b] == 0:
                        new_board = copy.deepcopy(board)
                        new_board[a][b] = 2
                        # switch player
                        v = min(v, self.search_agent(new_board, "PLAYER", start, time_limit, depth + 1, max_depth))
            if depth == 1:
                return '', v
            return v


class MiniMaxIndexPlayer(AbstractIndexPlayer):
    """MiniMax Index Player,
    this player is the opponent of the move player and need to return the indices on the board where to put 2.
    the goal of the player is to reduce move player score.
    implement get_indices function according to MiniMax algorithm, the value in minimax player value is only 2.
    (you can add helper functions as you want).
    """

    def __init__(self):
        AbstractIndexPlayer.__init__(self)
        # TODO: add here if needed

    def get_indices(self, board, value, time_limit) -> (int, int):
        start = time.time()
        depth = 1
        min_score = float('inf')
        index1, index2 = 0, 0
        while time.time() - start < time_limit - 0.1:
            for a in range(4):
                for b in range(4):
                    if time.time() - start >= time_limit - 0.1:
                        return index1, index2
                    if board[a][b] == 0:
                        # copy board
                        new_board = copy.deepcopy(board)
                        # add tile
                        new_board[a][b] = 2
                        # iterative deepening
                        temp = self.search(board, "PLAYER", start, time_limit, 1, depth)
                        if isinstance(temp, tuple):
                            score = temp[1]
                        else:
                            score = temp
                        if score < min_score:
                            index1, index2 = a, b
            depth += 1
        return index1, index2

    def search(self, board, agent, start, time_limit, depth, max_depth):
        chosen_move = None
        # evaluate on final depth
        if depth > max_depth or max_depth == 0:
            move, val = getHueristicVal(board, start, time_limit)
            return val
        if time.time() - start >= time_limit - 0.1:
            return float('-inf')

        # player turn
        if agent == "PLAYER":
            currMax = float('-inf')
            for move in Move:
                new_board, done, score = commands[move](board)
                if done:
                    # switch player
                    v = self.search(new_board, "RIVAL", start, time_limit, depth + 1, max_depth)
                    if v > currMax and depth == 1:
                        chosen_move = move
                    currMax = max(currMax, v)
            if depth == 1:
                return chosen_move, currMax
            return currMax

        # index turn
        else:
            v = float('inf')
            for a in range(4):
                for b in range(4):
                    if board[a][b] == 0:
                        new_board = copy.deepcopy(board)
                        new_board[a][b] = 2
                        # switch player
                        v = min(v, self.search(new_board, "PLAYER", start, time_limit, depth + 1, max_depth))
            if depth == 1:
                return '', v
            return v


# part C
class ABMovePlayer(AbstractMovePlayer):
    """Alpha Beta Move Player,
    implement get_move function according to Alpha Beta MiniMax algorithm
    (you can add helper functions as you want)
    """

    def __init__(self):
        AbstractMovePlayer.__init__(self)
        # TODO: add here if needed

    def get_move(self, board, time_limit) -> Move:
        start = time.time()
        res_move = Move.UP  # the base move
        max_score = float('-inf')
        depth = 1
        # iterative deepening
        while time.time() - start < time_limit - 0.1:
            move, score = self.search_agent(board, "PLAYER", start, time_limit, 1, depth, float('-inf'), float('inf'))
            if score > max_score:
                max_score = score
                res_move = move
            depth += 1
        return res_move

        # TODO: add here helper functions in class, if needed

    def search_agent(self, board, agent, start, time_limit, depth, max_depth, alpha, beta):
        chosen_move = None
        # evaluate on final depth
        if depth > max_depth or max_depth == 0:
            move, val = getHueristicVal(board, start, time_limit)
            return val
        if time.time() - start >= time_limit - 0.1:
            return float('-inf')

        # player turn
        if agent == "PLAYER":
            currMax = float('-inf')
            for move in Move:
                new_board, done, score = commands[move](board)
                if done:
                    # switch player
                    v = self.search_agent(new_board, "RIVAL", start, time_limit, depth + 1, max_depth, alpha, beta)
                    if v > currMax and depth == 1:
                        chosen_move = move
                    currMax = max(currMax, v)
                    if currMax >= beta:
                        return float('inf')
                    alpha = max(alpha, currMax)
            if depth == 1:
                return chosen_move, currMax
            return currMax

        # index turn
        else:
            v = float('inf')
            for a in range(4):
                for b in range(4):
                    if board[a][b] == 0:
                        new_board = copy.deepcopy(board)
                        new_board[a][b] = 2
                        # switch player
                        v = min(v,
                                self.search_agent(new_board, "PLAYER", start, time_limit, depth + 1, max_depth, alpha,
                                                  beta))
                        if v <= alpha:
                            return float('-inf')
                        beta = min(beta, v)
            if depth == 1:
                return '', v
            return v


# part D
class ExpectimaxMovePlayer(AbstractMovePlayer):
    """Expectimax Move Player,
    implement get_move function according to Expectimax algorithm.
    (you can add helper functions as you want)
    """

    def __init__(self):
        AbstractMovePlayer.__init__(self)
        # TODO: add here if needed

    def get_move(self, board, time_limit) -> Move:
        start = time.time()
        res_move = Move.UP  # the base move
        max_score = float('-inf')
        depth = 1
        # iterative deepening
        while time.time() - start < time_limit - 0.1:
            move, score = self.search_agent(board, "PLAYER", start, time_limit, 1, depth, 0, None)
            if score > max_score:
                max_score = score
                res_move = move
            depth += 1
        return res_move

    # TODO: add here helper functions in class, if needed

    def search_agent(self, board, agent, start, time_limit, depth, max_depth, probabilistic, probabilistic_value):
        chosen_move = None
        # evaluate on final depth
        if depth > max_depth or max_depth == 0:
            move, val = getHueristicVal(board, start, time_limit)
            return val
        if time.time() - start >= time_limit - 0.1:
            return float('-inf')

        if (probabilistic):
            min_2 = self.search_agent(board, "RIVAL", start, time_limit, depth + 1, max_depth, 0, 2)
            min_4 = self.search_agent(board, "RIVAL", start, time_limit, depth + 1, max_depth, 0, 4)
            v = (0.9 * min_2 + 0.1 * min_4)
            if depth == 1:
                return '', v
            return v

        # player turn
        if agent == "PLAYER":
            currMax = float('-inf')
            for move in Move:
                new_board, done, score = commands[move](board)
                if done:
                    # switch player
                    v = self.search_agent(new_board, None, start, time_limit, depth + 1, max_depth, 1, None)
                    if v > currMax and depth == 1:
                        chosen_move = move
                    currMax = max(currMax, v)
            if depth == 1:
                return chosen_move, currMax
            return currMax

        # index turn
        else:
            v = float('inf')
            for a in range(4):
                for b in range(4):
                    if board[a][b] == 0:
                        new_board = copy.deepcopy(board)
                        new_board[a][b] = probabilistic_value
                        # switch player
                        v = min(v, self.search_agent(new_board, "PLAYER", start, time_limit, depth + 1, max_depth, 0,
                                                     None))
            if depth == 1:
                return '', v
            return v


class ExpectimaxIndexPlayer(AbstractIndexPlayer):
    """Expectimax Index Player
    implement get_indices function according to Expectimax algorithm, the value is number between {2,4}.
    (you can add helper functions as you want)
    """

    def __init__(self):
        AbstractIndexPlayer.__init__(self)
        # TODO: add here if needed

    def get_indices(self, board, value, time_limit) -> (int, int):
        start = time.time()
        depth = 1
        min_score = float('inf')
        index1, index2 = 0, 0
        while time.time() - start < time_limit - 0.1:
            for a in range(4):
                for b in range(4):
                    if time.time() - start >= time_limit - 0.1:
                        return index1, index2
                    if board[a][b] == 0:
                        # copy board
                        new_board = copy.deepcopy(board)
                        # add tile
                        new_board[a][b] = value
                        # iterative deepening
                        temp = self.search(board, "PLAYER", start, time_limit, 1, depth, 0, None)
                        if isinstance(temp, tuple):
                            score = temp[1]
                        else:
                            score = temp
                        if score < min_score:
                            index1, index2 = a, b
            depth += 1
        return index1, index2

    def search(self, board, agent, start, time_limit, depth, max_depth, probabilistic, probabilistic_value):
        chosen_move = None
        # evaluate on final depth
        if depth > max_depth or max_depth == 0:
            move, val = getHueristicVal(board, start, time_limit)
            return val
        if time.time() - start >= time_limit - 0.1:
            return float('-inf')

        if (probabilistic):
            min_2 = self.search(board, "RIVAL", start, time_limit, depth + 1, max_depth, 0, 2)
            min_4 = self.search(board, "RIVAL", start, time_limit, depth + 1, max_depth, 0, 4)
            v = (0.9 * min_2 + 0.1 * min_4)
            if depth == 1:
                return '', v
            return v

        # player turn
        if agent == "PLAYER":
            currMax = float('-inf')
            for move in Move:
                new_board, done, score = commands[move](board)
                if done:
                    # switch player
                    v = self.search(new_board, None, start, time_limit, depth + 1, max_depth, 1, None)
                    if v > currMax and depth == 1:
                        chosen_move = move
                    currMax = max(currMax, v)
            if depth == 1:
                return chosen_move, currMax
            return currMax

        # index turn
        else:
            v = float('inf')
            for a in range(4):
                for b in range(4):
                    if board[a][b] == 0:
                        new_board = copy.deepcopy(board)
                        new_board[a][b] = probabilistic_value
                        # switch player
                        v = min(v, self.search(new_board, "PLAYER", start, time_limit, depth + 1, max_depth, 0, None))
            if depth == 1:
                return '', v
            return v


# part E
# Tournament
class ContestMovePlayer(AbstractMovePlayer):
    """Contest Move Player,
    implement get_move function as you want to compete in the Tournament
    (you can add helper functions as you want)
    """

    def __init__(self):
        AbstractMovePlayer.__init__(self)
        # TODO: add here if needed

    def get_move(self, board, time_limit) -> Move:
        start = time.time()
        res_move = Move.UP  # the base move
        max_score = float('-inf')
        depth = 1
        # iterative deepening
        while time.time() - start < time_limit - 0.1:
            move, score = self.search_agent(board, "PLAYER", start, time_limit, 1, depth, float('-inf'), float('inf'))
            if score > max_score:
                max_score = score
                res_move = move
            depth += 1
        return res_move

    # TODO: add here helper functions in class, if needed

    def search_agent(self, board, agent, start, time_limit, depth, max_depth, alpha, beta):
        chosen_move = None
        # evaluate on final depth
        if depth > max_depth or max_depth == 0:
            move, val = getHueristicVal(board, start, time_limit)
            return val
        if time.time() - start >= time_limit - 0.1:
            return float('-inf')

        # player turn
        if agent == "PLAYER":
            currMax = float('-inf')
            for move in Move:
                new_board, done, score = commands[move](board)
                if done:
                    # switch player
                    v = self.search_agent(new_board, "RIVAL", start, time_limit, depth + 1, max_depth, alpha, beta)
                    if v > currMax and depth == 1:
                        chosen_move = move
                    currMax = max(currMax, v)
                    if currMax >= beta:
                        return float('inf')
                    alpha = max(alpha, currMax)
            if depth == 1:
                return chosen_move, currMax
            return currMax

        # index turn
        else:
            v = float('inf')
            for a in range(4):
                for b in range(4):
                    if board[a][b] == 0:
                        new_board = copy.deepcopy(board)
                        new_board[a][b] = 2
                        # switch player
                        v = min(v,
                                self.search_agent(new_board, "PLAYER", start, time_limit, depth + 1, max_depth, alpha,
                                                  beta))
                        if v <= alpha:
                            return float('-inf')
                        beta = min(beta, v)
            if depth == 1:
                return '', v
            return v
