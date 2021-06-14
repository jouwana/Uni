from .graph_problem_interface import *
from .astar import AStar
from typing import Optional, Callable
import numpy as np
import math


class AStarEpsilon(AStar):
    """
    This class implements the (weighted) A*Epsilon search algorithm.
    A*Epsilon algorithm basically works like the A* algorithm, but with
    another way to choose the next node to expand from the open queue.
    """

    solver_name = 'A*eps'

    def __init__(self,
                 heuristic_function_type: HeuristicFunctionType,
                 within_focal_priority_function: Callable[[SearchNode, GraphProblem, 'AStarEpsilon'], float],
                 heuristic_weight: float = 0.5,
                 max_nr_states_to_expand: Optional[int] = None,
                 focal_epsilon: float = 0.1,
                 max_focal_size: Optional[int] = None):
        # A* is a graph search algorithm. Hence, we use close set.
        super(AStarEpsilon, self).__init__(heuristic_function_type, heuristic_weight,
                                           max_nr_states_to_expand=max_nr_states_to_expand)
        self.focal_epsilon = focal_epsilon
        if focal_epsilon < 0:
            raise ValueError(f'The argument `focal_epsilon` for A*eps should be >= 0; '
                             f'given focal_epsilon={focal_epsilon}.')
        self.within_focal_priority_function = within_focal_priority_function
        self.max_focal_size = max_focal_size

    def _init_solver(self, problem):
        super(AStarEpsilon, self)._init_solver(problem)

    def _extract_next_search_node_to_expand(self, problem: GraphProblem) -> Optional[SearchNode]:
        """
        Extracts the next node to expand from the open queue,
         by focusing on the current FOCAL and choosing the node
         with the best within_focal_priority from it.
        
        Find the minimum expanding-priority value in the `open` queue.
        Calculate the maximum expanding-priority of the FOCAL, which is
         the min expanding-priority in open multiplied by (1 + eps) where
         eps is stored under `self.focal_epsilon`."""
        if self.open.is_empty():
            return None
        
        minimumExpandingPriority = self.open.peek_next_node().expanding_priority
        maximumExpandingPriority = minimumExpandingPriority * (1+self.focal_epsilon)

        """
        Create the FOCAL by popping items from the `open` queue and inserting
         them into a focal list. Don't forget to satisfy the constraint of
         `self.max_focal_size` if it is set (not None)
         """

        FOCAL = []
        fe = minimumExpandingPriority
        while fe <= maximumExpandingPriority:
            if self.max_focal_size is not None:
                if len(FOCAL) == self.max_focal_size:
                    break
            if not self.open.is_empty():
                fe = self.open.peek_next_node().expanding_priority
                if fe <= maximumExpandingPriority:
                    FOCAL.append(self.open.pop_next_node())
            else:
                break

        """
        For each node (candidate) in the created focal, calculate its priority
         by calling the function `self.within_focal_priority_function` on it. 
        
        """
        arr = []
        for node in FOCAL:
            arr.append(self.within_focal_priority_function(node, problem, self))

        # find the minimum index value in arr, it will be the same as the index for
        # the focal node, as we appended them in order into the array
        index = np.argmin(arr)
        focalnode = FOCAL.pop(index)

        # return unused nodes to open
        for node in FOCAL:
            self.open.push_node(node)
            
        self.close.add_node(focalnode)
        
        return focalnode
