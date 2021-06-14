from .graph_problem_interface import *
from .best_first_search import BestFirstSearch
from typing import Optional, Callable


class AStar(BestFirstSearch):
    """
    This class implements the Weighted-A* search algorithm.
    A* algorithm is in the Best First Search algorithms family.
    """

    solver_name = 'A*'

    def __init__(self, heuristic_function_type: HeuristicFunctionType, heuristic_weight: float = 0.5,
                 max_nr_states_to_expand: Optional[int] = None,
                 open_criterion: Optional[Callable[[SearchNode], bool]] = None):
        """
        :param heuristic_function_type: The A* solver stores the constructor of the heuristic
                                        function, rather than an instance of that heuristic.
                                        In each call to "solve_problem" a heuristic instance
                                        is created.
        :param heuristic_weight: Used to calculate the f-score of a node using
                                 the heuristic value and the node's cost. Default is 0.5.
        """
        # A* is a graph search algorithm. Hence, we use close set.
        super(AStar, self).__init__(
            use_close=True, max_nr_states_to_expand=max_nr_states_to_expand, open_criterion=open_criterion)
        self.heuristic_function_type = heuristic_function_type
        self.heuristic_function = None
        self.heuristic_weight = heuristic_weight

    def _init_solver(self, problem):
        """
        Called by "solve_problem()" in the implementation of `BestFirstSearch`.
        The problem to solve is known now, so we can create the heuristic function to be used.
        """
        super(AStar, self)._init_solver(problem)
        self.heuristic_function = self.heuristic_function_type(problem)
        self.solver_name = f'{self.__class__.solver_name} (h={self.heuristic_function.heuristic_name}, w={self.heuristic_weight:.3f})'

    def _calc_node_expanding_priority(self, search_node: SearchNode) -> float:
        """
        Called by solve_problem() in the implementation of `BestFirstSearch`
         whenever just after creating a new successor node.
        Should calculate and return the f-score of the given node.
        This score is used as a priority of this node in the open priority queue.


        In Weighted-A* the f-score is defined by ((1-w) * cost) + (w * h(state)).
        
        """
        w = self.heuristic_weight
        g = search_node.g_cost

        return (1-w)*g + w*self.heuristic_function.estimate(search_node.state)

    def _open_successor_node(self, problem: GraphProblem, successor_node: SearchNode):
        """
        Called by solve_problem() in the implementation of `BestFirstSearch`
         whenever creating a new successor node.
        This method is responsible for adding this just-created successor
         node into the `self.open` priority queue, and may check the existence
         of another node representing the same state in `self.close`.

        
       In A*, in contrast to uniform-cost, a successor state might have an already closed node,
                  but still could be improved.
        """

        # if node is in open queue
        if self.open.has_state(successor_node.state):
            # get the node currently in open
            already_found_node_with_same_state = self.open.get_node_by_state(successor_node.state)
            # check if the new node has a lower g cost
            if already_found_node_with_same_state.g_cost > successor_node.g_cost:
                # remove the old node from the open queue (so we can add the new one late as there
                # is no update key function i could find)
                self.open.extract_node(already_found_node_with_same_state)

        # if node is not in open, check if its in the close queue
        elif self.close.has_state(successor_node.state):
            # get the node currently in close
            already_closed_node_with_same_state = self.close.get_node_by_state(successor_node.state)
            # check if the new node has a lower g cost
            if already_closed_node_with_same_state.g_cost > successor_node.g_cost:
                # remove the old node from the close queue (so we can add the new one to the open
                # queue later, and we cant have the same state in close and open at the same time)
                self.close.remove_node(already_closed_node_with_same_state)


        # if node is not in open or close (or maybe it used to be but we removed it from there)
        if not self.open.has_state(successor_node.state) and not self.close.has_state(successor_node.state):
            # add new node to open
            self.open.push_node(successor_node)


