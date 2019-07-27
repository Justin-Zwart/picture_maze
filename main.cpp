//
// main.cpp
//


#include <array>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <random>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

vector<vector<int>> blur_picture(Mat& image, int x_size, int y_size, int maze_x_size, int maze_y_size);
vector<vector<vector<bool>>> edge_detection(vector<vector<int>> image);
void maze_generation(vector<vector<bool>>& visited_cells, vector<vector<vector<bool>>>& walls, vector<vector<vector<bool>>>& edges, int x_size, int y_size);
void area_change(Mat& image, array<int, 2> coor_1, array<int, 2> coor_2, int colour);


int main(int argc, char** argv)
{
    random_device random_seed;
    mt19937 generator(random_seed());
    uchar red_val, green_val, blue_val, grayscale_value;
    Vec3b colour;
    int maze_x_size, maze_y_size, x_size, y_size;
    int grayscale_int_val;
    vector<vector<int>> blurred_picture;
    vector<vector<vector<bool>>> edges, walls;
    vector<vector<bool>> visited_cells;
    vector<bool> empty_vector;

    // setup
    // Read the image file
    Mat image = imread("C:/Users/justi/Pictures/Test Cases/smiley_pic.jpg"); // put path to picture here

    // Check for failure
    if (image.empty())
    {
        cout << "Could not open or find the image \n";
        cin.get(); //wait for any key press
        return -1;
    }

    // define size
    x_size = image.cols;
    y_size = image.rows;

    // input maze size
    cout << "How many pixels high would you like the image?\n";
    cin >> maze_y_size;

    // loop until 0 < maze_y_size <= y_size to prevent division by zero or other errors
    while (maze_y_size <= 0 || maze_y_size > y_size)
    {
        cout << "Please enter a value where 0 < maze_y_size <= y_size.\n";
        cin >> maze_y_size;
    }

    maze_x_size = x_size * maze_y_size / y_size;  // make maze_y_size proportional to maze_x_size

    // ensure that maze x size doesn't get rounded to zero
    if (maze_x_size == 0)
        maze_x_size = 1;

    // iterate through all pixels, convert them to grayscale
    for (int x = 0; x < x_size; x++)
    {
        for (int y = 0; y < y_size; y++)
        {
            // get rgb values
            colour = image.at<Vec3b>(y, x);

            red_val = colour[0];
            green_val = colour[1];
            blue_val = colour[2];

            // find luminosity of pixel to represent grayscale value
            grayscale_value = (red_val * 0.21) + (green_val * 0.72) + (blue_val * 0.07);

            grayscale_int_val = grayscale_value;

            // change pixel to new grayscale pixel
            image.at<Vec3b>(y, x)[0] = grayscale_value;
            image.at<Vec3b>(y, x)[1] = grayscale_value;
            image.at<Vec3b>(y, x)[2] = grayscale_value;
        }
    }

    // call on functions to prepare for maze generation
    blurred_picture = blur_picture(image, x_size, y_size, maze_x_size, maze_y_size);

    edges = edge_detection(blurred_picture);

    // create list of walls for maze generation
    // set to same size as edges, then loop thrpugh and change all values to true
    walls = edges;
    for (int i = 0; i < 2; i++)
    {
        for (int x = 0; x < edges[i].size(); x++)
        {
            for (int y = 0; y < edges[i][x].size(); y++)
            {
                walls[i][x][y] = true;
            }
        }
    }

    // set up list of cells required for maze generation
    for (int x = 0; x < maze_x_size; x++)
    {
        visited_cells.push_back(empty_vector);
        for (int y = 0; y < maze_y_size; y++)
        {
            visited_cells[x].push_back(false);
        }
    }

    // generate maze
    maze_generation(visited_cells, walls, edges, maze_x_size, maze_y_size);

    // create image to display maze in
    Mat maze(10*maze_y_size + 2, 10 * maze_x_size + 2, CV_8UC3, Scalar(255, 255, 255));

    // create maze
    // add border to maze
    area_change(maze, { 10, 0 }, { 10 * maze_x_size, 1 }, 0);
    area_change(maze, { 0, 0 }, { 1, 10 * maze_y_size + 1 }, 1);
    area_change(maze, { 10 * maze_x_size, 0 }, { 10 * maze_x_size + 1, 10 * maze_y_size + 1 }, 0);
    area_change(maze, { 0, 10 * maze_y_size }, { 10 * maze_x_size - 9, 10 * maze_y_size + 1 }, 0);


    // add vertical edges to maze
    for (int x = 0; x < walls[0].size(); x++)
    {
        for (int y = 0; y < walls[0][0].size(); y++)
        {
            if (walls[0][x][y])
            {
                // make wall thicker if it is an edge
                if (!edges[0][x][y])
                    area_change(maze, { 10 * x + 10, 10 * y + 2 }, { 10 * x + 11, 10 * y + 9 }, 0);
                else
                    area_change(maze, { 10 * x + 9, 10 * y + 1 }, { 10 * x + 12, 10 * y + 10 }, 0);
            }
        }
    }

    // add horizantal edges to maze
    for (int x = 0; x < walls[1].size(); x++)
    {
        for (int y = 0; y < walls[1][0].size(); y++)
        {
            if (walls[1][x][y])
            {
                // make wall thicker if it is an edge
                if (!edges[1][x][y])
                    area_change(maze, { 10 * x + 2, 10 * y + 10 }, { 10 * x + 9, 10 * y + 11 }, 0);
                else
                    area_change(maze, { 10 * x + 1, 10 * y + 9 }, { 10 * x + 10, 10 * y + 12 }, 0);
            }
        }
    }

    // add dots at intersections+
    for (int x = 1; x < maze_x_size; x++)
    {
        for (int y = 1; y < maze_y_size; y++)
        {
            area_change(maze, { 10 * x, 10 * y }, { 10 * x + 1, 10 * y + 1 }, 0);
        }
    }

    // create a window, display image
    namedWindow("Maze"); 
    imshow("Maze", maze);

    // print image to path
    imwrite("C:/Users/justi/Pictures/maze.jpg", maze);

    // wait for keypress in window, then end program
    waitKey(0); 
    destroyWindow("Maze");

    return 0;
}


vector<vector<int>> blur_picture(Mat& image, int x_size, int y_size, int maze_x_size, int maze_y_size)
{
    vector<int> empty_vector;
    vector<vector<int>> empty_vec_of_vecs, blurred_image;
    vector<vector<vector<int>>> blur_sizes;
    int x_blur_size, y_blur_size;
    int x_count, y_count, x_pos, y_pos;

    // create matrix representing how many pixels go into each section for the blur
    x_blur_size = floor(x_size / maze_x_size);
    y_blur_size = floor(y_size / maze_y_size);

    for (int x = 0; x < maze_x_size; x++)
    {
        blur_sizes.push_back(empty_vec_of_vecs);
        for (int y = 0; y < maze_y_size; y++)
        {
            blur_sizes[x].push_back(empty_vector);

            // add x and y blur sizes
            blur_sizes[x][y].push_back(x_blur_size);
            blur_sizes[x][y].push_back(y_blur_size);
        }
    }

    // add remainder to x size
    for (int x = 0; x < x_size % maze_x_size; x++)
    {
        for (int y = 0; y < maze_y_size; y++)
        {
            blur_sizes[x][y][0] += 1;
        }
    }

    // add remainder to y size
    for (int y = 0; y < y_size % maze_y_size; y++)
    {
        for (int x = 0; x < maze_x_size; x++)
        {
            blur_sizes[x][y][1] += 1;
        }
    }

    // create blurred image vectors
    for (int x = 0; x < blur_sizes.size(); x++)
    {
        blurred_image.push_back(empty_vector);
        for (int y = 0; y < blur_sizes[x].size(); y++)
        {
            blurred_image[x].push_back(0);
        }
    }

    // add pixels to the blurred image
    x_count = 0;
    y_count = 0;
    x_pos = 0;
    y_pos = 0;

    for (int x = 0; x < x_size; x++)
    {
        // shift to a new section if x value is exceeded 
        if (x_count >= blur_sizes[x_pos][0][0])
        {
            x_pos += 1;
            x_count = 0;
        }

        for (int y = 0; y < y_size; y++)
        {
            // shift to a new section if y value is exceeded
            if (y_count >= blur_sizes[0][y_pos][1])
            {
                y_pos += 1;
                y_count = 0;
            }

            blurred_image[x_pos][y_pos] += image.at<Vec3b>(y, x)[0];

            y_count += 1;
        }

        // increment and reset variables
        x_count += 1;
        y_count = 0;
        y_pos = 0;
    }

    // divide each section by the number of pixels in that section
    for (int x = 0; x < blurred_image.size(); x++)
    {
        for (int y = 0; y < blurred_image[x].size(); y++)
        {
            blurred_image[x][y] = blurred_image[x][y] / (blur_sizes[x][y][0] * blur_sizes[x][y][1]);
        }
    }

    return blurred_image;
}


vector<vector<vector<bool>>> edge_detection(vector<vector<int>> image)
{
    vector<vector<vector<bool>>> edges;
    vector<vector<int>> differences_x, differences_y;
    vector<int> empty_vector;
    vector<bool> empty_bool_vector;
    vector<vector<bool>> empty_vec_of_bool_vecs;
    int difference, average_difference, edge_requirement, step;
    
    average_difference = 0;

    // find the differences between the pixels and pixels beside them
    for (int x = 0; x < image.size() - 1; x++)
    {
        differences_x.push_back(empty_vector);
        for (int y = 0; y < image[x].size(); y++)
        {
            step = x + 1;  // cast value to a wider type to avoid overflow
            difference = abs(image[x][y] - image[step][y]);
            average_difference += difference;
            differences_x[x].push_back(difference);
        }
    }

    // find the differences between the pixels above and below them
    for (int x = 0; x < image.size(); x++)
    {
        differences_y.push_back(empty_vector);
        for (int y = 0; y < image[x].size() - 1; y++)
        {
            step = y + 1;  // cast value to a wider type to avoid overflow
            difference = abs(image[x][y] - image[x][step]);
            average_difference += difference;
            differences_y[x].push_back(difference);
        }
    }

    // divide sum of all edges by (2xy - x - y) to get average difference
    if (average_difference != 0)
        average_difference = average_difference / (2 * image.size() * image[0].size() - image.size() - image[0].size());

    // set requirement to be an edge
    edge_requirement = average_difference * 2.5;

    // add horizantal edges to a list of booleans
    edges.push_back(empty_vec_of_bool_vecs);
    for (int x = 0; x < differences_x.size(); x++)
    {
        edges[0].push_back(empty_bool_vector);
        for (int y = 0; y < differences_x[x].size(); y++)
        {
            // if edge is greater than edge requirement, 
            if (differences_x[x][y] >= edge_requirement)
                edges[0][x].push_back(true);
            else
                edges[0][x].push_back(false);
        }
    }

    // add vertical edges to a list of booleans
    edges.push_back(empty_vec_of_bool_vecs);
    for (int x = 0; x < differences_y.size(); x++)
    {
        edges[1].push_back(empty_bool_vector);
        for (int y = 0; y < differences_y[x].size(); y++)
        {
            // if edge is greater than edge requirement, 
            if (differences_y[x][y] >= edge_requirement)
                edges[1][x].push_back(true);
            else
                edges[1][x].push_back(false);
        }
    }

    return edges;
}


void maze_generation(vector<vector<bool>>& visited_cells, vector<vector<vector<bool>>>& walls, vector<vector<vector<bool>>>& edges, int x_size, int y_size)
{
    vector<array<int, 2>> location_history;
    vector<array<int, 3>> valid_locations;
    array<int, 2> location;
    int step, selected_cell;
    random_device random_seed;
    mt19937 generator(random_seed());
    uniform_int_distribution<int> start_x(0, x_size - 1);
    uniform_int_distribution<int> start_y(0, y_size - 1);

    // set up starting location
    location_history.push_back({ 0, 0 });
    location_history[0][0] = start_x(generator);
    location_history[0][1] = start_y(generator);

    while (location_history.size() > 0)
    {
        // find current location
        location = location_history[location_history.size() - 1];

        // set cell to visited
        visited_cells[location[0]][location[1]] = true;

        // check if cell to left is a valid location
        if (location[0] != 0)
        {
            step = location[0] - 1;  // cast value to a wider type to avoid overflow
            if (visited_cells[step][location_history[location_history.size() - 1][1]] == false)
            {
                valid_locations.push_back({ 0, 0, 0 });
                valid_locations[valid_locations.size() - 1][0] = location[0] - 1;
                valid_locations[valid_locations.size() - 1][1] = location[1];

                // check if it is an edge
                if (edges[0][step][location[1]])
                    valid_locations[valid_locations.size() - 1][2] = 1;
                else
                    valid_locations[valid_locations.size() - 1][2] = 0;
            }
        }

        // check if cell to right is a valid location
        if (location[0] != x_size - 1)
        {
            step = location[0] + 1;  // cast value to a wider type to avoid overflow
            if (visited_cells[step][location[1]] == false)
            {
                valid_locations.push_back({ 0, 0, 0 });
                valid_locations[valid_locations.size() - 1][0] = location[0] + 1;
                valid_locations[valid_locations.size() - 1][1] = location[1];

                // check if it is an edge
                if (edges[0][location[0]][location[1]])
                    valid_locations[valid_locations.size() - 1][2] = 0;
                else
                    valid_locations[valid_locations.size() - 1][2] = 0;
            }
        }

        // check if cell below it is a valid location
        if (location[1] != 0)
        {
            step = location[1] - 1;  // cast value to a wider type to avoid overflow
            if (visited_cells[location[0]][step] == false)
            {
                valid_locations.push_back({ 0, 0, 0 });
                valid_locations[valid_locations.size() - 1][0] = location[0];
                valid_locations[valid_locations.size() - 1][1] = location[1] - 1;

                // check if it is an edge
                if (edges[1][location[0]][step])
                    valid_locations[valid_locations.size() - 1][2] = 1;
                else
                    valid_locations[valid_locations.size() - 1][2] = 0;
            }
        }

        // check if cell above it is a valid location
        if (location[1] != y_size - 1)
        {
            step = location[1] + 1;  // cast value to a wider type to avoid overflow
            if (visited_cells[location[0]][step] == false)
            {
                valid_locations.push_back({ 0, 0, 0 });
                valid_locations[valid_locations.size() - 1][0] = location[0];
                valid_locations[valid_locations.size() - 1][1] = location[1] + 1;

                // check if it is an edge
                if (edges[1][location[0]][location[1]])
                    valid_locations[valid_locations.size() - 1][2] = 1;
                else
                    valid_locations[valid_locations.size() - 1][2] = 0;
            }
        }

        // if no valid cells, backtrack
        if (valid_locations.size() == 0)
            location_history.pop_back();
        else
        {
            // create
            uniform_int_distribution<int> randint(0, valid_locations.size() - 1);

            // repeat until selected wall is not an edge, or it tries to select a wall three times
            for (int i = 0; i < 3; i++)
            {
                selected_cell = randint(generator);

                if (valid_locations[selected_cell][2] == 0)
                    break;
            }

            // destroy selected wall
            if (location[0] < valid_locations[selected_cell][0])
                walls[0][location[0]][location[1]] = false;
            else if (location[1] < valid_locations[selected_cell][1])
                walls[1][location[0]][location[1]] = false;
            else if (location[0] > valid_locations[selected_cell][0])
                walls[0][valid_locations[selected_cell][0]][valid_locations[selected_cell][1]] = false;
            else
                walls[1][valid_locations[selected_cell][0]][valid_locations[selected_cell][1]] = false;

            // put new location in history
            location_history.push_back({ valid_locations[selected_cell][0], valid_locations[selected_cell][1] });

            // reset valid locations
            valid_locations.clear();
        }
    }
}


void area_change(Mat& image, array<int, 2> coor_1, array<int, 2> coor_2, int colour)
{
    // loop through area and change each value to desired colour
    for (int x = coor_1[0]; x <= coor_2[0]; x++)
    {
        for (int y = coor_1[1]; y <= coor_2[1]; y++)
        {
            image.at<Vec3b>(y, x) = colour;
        }
    }
}
