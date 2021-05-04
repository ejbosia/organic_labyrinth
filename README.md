# organic_labyrinth

Implementation of Organic Maxes and Labyrinths paper: http://www.dgp.toronto.edu/~karan/pdf/mazes.pdf

## TODO
 - [x] Basic algorithm
 - [ ] Add boundary
 - [ ] CLI parameters
 - [ ] Variable scaling function
 - [ ] User interface
 - [ ] User drawn boundaries

## Installation instructions

The program uses these libraries:
 - **numpy**: used to hold point data
 - **numba**: used to speed up processing
 - **matplotlib**: used for plotting
 - **shapely**: used for geometry processing

These libraries can be installed using the requirements.txt file.

## Execution instructions

*Process needs to be updated* ~ process flow is still in development. The code right now runs a preset shape, and outputs a gif of the maze generation.

```bash
python3 labyrinth.py
```

## Example Result

This is an example result using a 4x4 square over 200 steps with no defined boundary.

![Example Maze](https://github.com/ejbosia/organic_labyrinth/blob/9b5d646c14125f91bcd34b21adcc086279715d94/test2.gif)

Configs used in this example:
- A = 0.006
- B = 0.05
- F = 0.1
- k0 = 1.0 
- k1 = 10.0
- kmin = 0.2
- kmax = 0.6
- D = 1.0
