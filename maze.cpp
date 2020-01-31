#include <vector>
#include <iostream>
#include <cmath>
#include "maze.h"
#include "point.h"
#include <cstdlib>
using namespace std;

const float playerAcc = .006, DECAY = 1.1, MOUSE_SENSE = 5, PI = 3.141592;
const int maxAngle = 10000;

Maze::Maze(int x, int y, Point origin) {
    ori = origin;
    xSize = x;
    ySize = y;
    player = origin;
    playerV = {0, 0};
    moves = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};//{>, v, <, ^}
    maze = emptyMaze(x, y);
    enemies = {};
    dead = false;
    win = false;
    cout << "Generating maze..." << endl;
    populateMaze(origin);
    setTile(Point(0, 1), true);
    int i = ySize - 2;
    cout << "Making exits..." << endl;
    while (true) {
        if (mazeAt(Point(xSize - 2, i))) {
            setTile(Point(xSize - 1, i), true);
            end = Point(xSize - 1, i);
            break;
        } else {
            i--;
        }
    }
    cout << "Placing enemies..." << endl;
//    for (int i = 0; i < (x * x / 49); i++) {
//        Point ij((rand() % (x - 2) + 1), (rand() % (x - 2) + 1));
//        while (!mazeAt(ij)) {
//            ij = Point((rand() % (x - 2) + 1), (rand() % (x - 2) + 1));
//        }
//        enemies.emplace_back(ij);
//    }
    for (int i = 0; i < (xSize * xSize / 35); i++) {
        Point ij((rand() % (x - 2) + 1), (rand() % (x - 2) + 1));
        while (true) {
            if (!mazeAt(ij)) {
                break;
            }
            ij = Point((rand() % (x - 2) + 1), (rand() % (x - 2) + 1));

        }
        setTile(ij, true);
    }
    cout << "Your maze is ready!" << endl;
}

int Maze::getX() {
    return xSize;
}
int Maze::getY() {
    return ySize;
}

int Maze::getFace() {
    return player.facing;
}

Vector2i Maze::getPlayer() {
    return Vector2i(player.xI(), player.yI());
}

Vector2f Maze::getPlayerOff() {
    float *f;
    return Vector2f(modff(player.xF(), f), modff(player.yF(), f));
}

Vector2f Maze::getPlayerF() {
    return Vector2f(player.xF(), player.yF());
}

vector<vector<bool>> Maze::getMaze() {
    return maze;
}

vector<vector<bool>> Maze::emptyMaze(int x, int y) {
    vector<vector<bool>> output = {};
    for(int i = 0; i < y; i++) {
        vector<bool> row = {};
        for(int j = 0; j < x; j++) {
            row.emplace_back(false);
        }
        output.emplace_back(row);
    }
    return output;
}

void Maze::populateMaze(Point origin) {
    getPathRand(origin);
    for (int i = 1; i < xSize - 1; i++) {
        for (int j = 1; j < ySize - 1; j++) {
            if (mazeAt(Point(i, j))) {
                getPathRand(Point(i, j));
            }
        }
    }
}

bool Maze::mazeAt(Point target) {
    return maze[target.xI()][target.yI()];
}

bool Maze::mazeAt(Vector2i target) {
    return maze[target.x][target.y];
}

bool Maze::inMaze(Point target) {
    return target.verify() and target.xI() < xSize - 1 and target.yI() < ySize - 1;
}

bool Maze::validSpace(Point target) {
    if (!inMaze(target) or mazeAt(target)) {
        return false;
    }
    int count = 0;
    vector<Point> gotMoves = getMoves(target);
    for (int i = 0; i < gotMoves.size(); i++) {
        if (mazeAt(gotMoves[i])) {
            count++;
        }
    }
    return inMaze(target) and count == 1;
}

vector<Point> Maze::getMoves(Point target) {
    vector<Point> final = {};
    for (int i = 0; i < moves.size(); i++) {
        final.emplace_back(target.vectAdd(moves[i]));
    }
    return final;
}

void Maze::setTile(Point target, bool value) {
    maze[target.xI()][target.yI()] = value;
}

Point Maze::step(Point target) {
    vector<Point> movesPossible = {};
    for (int i = 0; i < 4; i++) {
        if (validSpace(target.vectAdd(moves[i]))) {
            movesPossible.emplace_back(target.vectAdd(moves[i]));
        }
    }
    if (movesPossible.empty()) {
        return target;
    }
    return movesPossible[rand() % movesPossible.size()];
}

vector<Point> Maze::getPathRand(Point posI) {
    setTile(posI, true);
    vector<Point> path = {posI};
    while (true) {
        Point step = this->step(path[path.size() - 1]);
        if (step == path[path.size() - 1]) {
            break;
        } else {
            path.emplace_back(step);
            setTile(step, true);
        }
    }
    return path;
}

int sign(int i) {
    if (i > 0) return 1;
    return -1;
}

bool Maze::los(Point target) {
    Vector2i play(player.xI(), player.yI()), targ(target.xI(), target.yI()), diff(targ-play), i(0, 0);
    if (diff.x == 0) {
        i.y = sign(diff.y);
    } else if (diff.y == 0) {
        i.x = sign(diff.x);
    } else {
        return false;
    }
    while (play != targ) {
        if (!mazeAt(play)) {
            return false;
        }
        play += i;
    }
    return true;
}

float Maze::distance(Vector2i v) {
    return (float)sqrt(pow(v.x - player.x, 2) + pow(v.y - player.y, 2));
}

void Maze::ins(vector<Vector2i> &v, Vector2i targ) {
    if (v.empty()) {
        v.emplace_back(targ);
        return;
    }
    if (!mazeAt(targ)) return;
    float dis = distance(targ);
    int i;
    for (i = 0; distance(v[i]) < dis && i < v.size(); i++) {
        if (v[i] == targ) {
            return;
        }
    }
    v.insert(v.begin() + i, targ);
}

void Maze::insWalls(vector<Vector2i> &v, Vector2i targ) {
    vector<Vector2i> vTarg;
    for (Vector2i v : vector<Vector2i>({Vector2i(1, 0), Vector2i(-1, 0), Vector2i(0, 1), Vector2i(0, -1)})) {
        vTarg.emplace_back(targ + v);
    }
    for (Vector2i vec : vTarg) {
        if (mazeAt(vec)) return;
        if (v.empty()) {
            v.emplace_back(vec);
            continue;
        }
        float dis = distance(vec);
        int i;
        for (i = 0; distance(v[i]) < dis && i < v.size(); i++) {
            if (v[i] == targ) {
                return;
            }
        }
        v.insert(v.begin() + i, targ);
    }
}

vector<Vector2i> Maze::los() {
    return {Vector2i(2, 2)};
//    vector<Vector2i> final = {getPlayer()};
//    for (Vector2i v : vector<Vector2i>({Vector2i(1, 0), Vector2i(-1, 0), Vector2i(0, 1), Vector2i(0, -1)})) {
//        for (Vector2i pos = getPlayer() + v; inMaze(Point(pos)) && pos.x != end.x && pos.y != end.y && mazeAt(pos); pos += v) {
//            ins(final, pos);
//            ins(final, pos - Vector2i(v.y, v.x));
//            ins(final, pos + Vector2i(v.y, v.x));
//        }
////        Vector2i ph = final[final.size() - 1] + v;
////        if (ph.x > 0) final.emplace_back(ph);
//    }
//    return final;
}

void Maze::mazePrint() {
    for (int i = 0; i < maze.size(); i++) {
        for(int j = 0; j < maze[i].size(); j++) {
            if(maze[i][j]) {
                cout << "  ";
            }else {
                cout << "# ";
            }
        }
        cout << endl;
    }
}

float Maze::angle(Vector2f loc) {
    int quad = 0;
    if (loc.x < 0 && loc.y > 0) {
        quad = maxAngle / 4;
    } else if (loc.x < 0 && loc.y < 0) {
        quad = maxAngle / 2;
    } else if (loc.x > 0 && loc.y < 0) {
        quad = 3 * maxAngle / 4;
    }
    return (maxAngle / 2 + quad + maxAngle * atan(loc.x / loc.y)/(PI * 2));
}

float Maze::xFromAngle(float angle, Vector2f screen) {
    return (screen.x / 2) + ((screen.x / 2) * (player.facing - angle) / FOV);
}

float Maze::yFromDist(float dist, Vector2f screen) {
    float ret = maxAngle * atan(1 / (2*dist)) / (2 * PI);
    if (ret > FOV) {
        return screen.y / 2;
    }
    return (screen.y / 2) - ((screen.y / 2) * ret / FOV);
}

vector<Vector2f> Maze::corners(int x, int y) {
    Vector2f base(x - player.x, y - player.y);
    vector<Vector2f> ret;
    ret.emplace_back(Vector2f(base.x, base.y));
    ret.emplace_back(Vector2f(base.x + 1, base.y ));
    ret.emplace_back(Vector2f(base.x + 1, base.y + 1));
    ret.emplace_back(Vector2f(base.x + 0, base.y + 1));
    return ret;
}

vector<ConvexShape> Maze::drawWall(int x, int y, Vector2f screen) {
    cout << screen.x << endl << endl;
//    cout << xFromAngle(player.facing, screen) << endl << endl;
    vector<ConvexShape> ret;
    vector<Vector2f> descCorn;
    for (Vector2f v : this->corners(x, y)) {
        descCorn.emplace_back(Vector2f((float)sqrt(pow(v.x , 2) + pow(v.y, 2)), angle(v)));
    }
    int ind = 0;
    for (int i=1; i<4; i++) {
        if (descCorn[i].x < descCorn[ind].x) {
            ind = i;
        }
    }
    descCorn = {descCorn[(ind + 1) % 4], descCorn[(ind + 2) % 4], descCorn[(ind + 3) % 4]};
    vector<Vector2f> points;
    //descCorn proper
    for (Vector2f v : descCorn) {
        float x = xFromAngle(v.y, screen);
        if (abs(x - screen.x / 2) < screen.x / 2) {
            points.emplace_back(Vector2f(x, yFromDist(v.x, screen)));
        }
    }
    for (int i=0; i<points.size(); i++) {
        Vector2f v = points[i];
        cout << "(" << v.x << ", " << v.y << ") from (" << descCorn[i].x << ", " << descCorn[i].y << ") | ";
    }
    cout << endl;
    if (points.size() > 1) {
        for (int i = 0; i < points.size() - 1; i++) {
            ConvexShape ph(4);
            ph.setFillColor(Color::Red);
            float screenX = screen.y / 2;
            ph.setPoint(0, Vector2f(points[i].x, screenX + points[i].y));
            ph.setPoint(1, Vector2f(points[i].x, screenX - points[i].y));
            ph.setPoint(2, Vector2f(points[i + 1].x, screenX - points[i + 1].y));
            ph.setPoint(3, Vector2f(points[i + 1].x, screenX + points[i + 1].y));
            ret.emplace_back(ph);
        }
    }
    return ret;
}

bool Maze::enemyAt(Point target) {
    for (int i = 0; i < enemies.size(); i++) {
        if (enemies[i] == target) {
            return true;
        }
    }
    return false;
}

void Maze::enemyMove() {
    for (int i = 0; i < enemies.size(); i++) {
        vector<Point> gotMoves = getMoves(enemies[i]), movesPoss = {};
        for (int j = 0; j < 4; j++) {
            if (mazeAt(gotMoves[j]) and inMaze(gotMoves[j])) {
                movesPoss.emplace_back(gotMoves[j]);
            }
        }
        enemies[i] = movesPoss.at(rand() % movesPoss.size());
        if (enemies[i] == Point(player.xI(), player.yI())) {
            dead = true;
        }
    }
}

void Maze::playerMove() {
    player.x += playerV.x;
//    if (!maze[player.xI()][player.yI()]) {
//        player.x -= playerV.x;
//        playerV = {0, playerV.y};
//    }
    player.y += playerV.y;
//    if (!maze[player.xI()][player.yI()]) {
//        player.y -= playerV.y;
//        playerV = {playerV.x, 0};
//    }
    playerV.x /= DECAY;
    playerV.y /= DECAY;
}

void Maze::update(int toTurn) {
    player.turn(-toTurn * MOUSE_SENSE);
    playerMove();
    if (enemyAt(Point(player.xF(), player.yF())) and !win) {
        dead = true;
    }
    if (player.xI() == end.xI() and !dead) {
        win = true;
    }
}

Vector2f Maze::rightDir() {
    return Vector2f(cos(2 * player.facing * PI / maxAngle), -sin(2 * player.facing * PI / maxAngle));
}

Vector2f Maze::downDir() {
    return Vector2f(sin(2 * player.facing * PI / maxAngle), cos(2 * player.facing * PI / maxAngle));
}

void Maze::left() {
    if (!win) playerV -= rightDir() * playerAcc;
}
void Maze::right() {
    if (!win) playerV += rightDir() * playerAcc;
}
void Maze::up() {
    if (!win) playerV -= downDir() * playerAcc;
}
void Maze::down() {
    if (!win) playerV += downDir() * playerAcc;
}

void Maze::stop() {
    playerV = {0, 0};
}


