#pragma once

#include <rckid/app.h>
#include <rckid/graphics/canvas.h>
#include <rckid/ui/header.h>
#include <rckid/ui/pause.h>

namespace rckid {

    /** A simple tetris game. 

        TODO

        - calculate score, print level, etc. 
        - move down, 
        - figure out game over
        - add effects? 
        - add game over 
        
     */
    class Tetris : public GraphicsApp<Canvas<ColorRGB>> {
    public:

        static void run() {
            Tetris t;
            t.resetGame();
            t.loop();
        }

    protected:

        Tetris(): GraphicsApp{Canvas<Color>{320, 240}} {}

        void update() override {
            // handle back button
            GraphicsApp::update();
            bool goDown = false;
            if (--countdown_ == 0) {
                goDown = true;
                countdown_ = speed_;
            }
            if (btnPressed(Btn::A))
                pause();
            // TODO if we should, go down automatically
            if (btnPressed(Btn::Up))
                rotate();
            if (btnDown(Btn::Down)) {
                if (allowDown_)
                    goDown = true;
            } else {
                allowDown_ = true;
            }
            // check left & right
            if (btnPressed(Btn::Left)) {
                if (validate(cur_, x_ - 1, y_))
                    --x_;
                // else rumble
            }
            if (btnPressed(Btn::Right)) {
                if (validate(cur_, x_ + 1, y_))
                    ++x_;
                // else rumble
            }
            if (goDown) {
                if (validate(cur_, x_, y_ + 1)) {
                    ++y_;
                } else {
                    addToGrid(cur_, x_, y_);
                    compactRows(y_);
                    spawn();
                    //if (!validate(cur_, x_, y_))
                    //  gameOver();
                }
            }
        }

        void draw() override {
            // clear
            g_.fill();
            int startX = (320 - PLAY_WIDTH * 10) / 2;
            int startY = 238 - PLAY_HEIGHT * 10;
            g_.fill(color::DarkGray, Rect::XYWH(startX - 2, startY-2, 10 * PLAY_WIDTH + 3, 10 * PLAY_HEIGHT + 3));
            // draw the playgrid
            for (int y = 0; y < PLAY_HEIGHT; ++y)
                for (int x = 0; x < PLAY_WIDTH; ++x)
                    drawTile(startX + x * 10, startY + y * 10, grid(x, y));
            // draw the current tetromino
            drawTetromino(startX + x_ * 10, startY + y_ * 10, cur_);
            drawTetromino(250, 100, next_);

            // and draw the header
            Header::drawOn(g_);
        }

    private:

        static constexpr uint8_t NUM_COLORS = 6;

        static constexpr Coord PLAY_WIDTH = 10;
        static constexpr Coord PLAY_HEIGHT = 22;

        /** Tetromino. 
         
            Can rotate, can move down, 
         */
        class Tetromino {
        public:
            enum class Kind {
                Beam,
                Square, 
                T,
                L1, 
                L2,
                Stairs1, 
                Stairs2,
            };

            /** Returns the size of tetromino grid. This is also the part of the grid that will be rotated. 
             */
            static constexpr Coord gridSize(Kind kind) {
                if (kind == Kind::Beam || kind == Kind::Square)
                    return 4;
                return 3;
            }

            Kind kind() const { return kind_; }

            uint8_t grid(int x, int y) const {
                return grid_[y][x];
            }

            /** Randomizes the tetromino's color and kind. 
             */
            void randomize() {
                kind_ = static_cast<Kind>(random() % (static_cast<uint32_t>(Kind::Stairs2) + 1));
                uint8_t color = (random() % (NUM_COLORS - 1)) + 1;
                for (unsigned x = 0; x < 4; ++x)
                    for (unsigned y = 0; y < 4; ++y)
                        grid_[y][x] = tetrominos_[static_cast<unsigned>(kind_)][y][x] ? color : 0;
            }


            /** Rotates the tetromino by 90 degrees. 
             
                All rotations go clockwise. 
             */
            Tetromino rotate() {
                Tetromino result{};
                result.kind_ = kind_;
                unsigned max = gridSize(kind_);
                for (unsigned x = 0; x < max; ++x)
                    for (unsigned y = 0; y < max; ++y)
                        result.grid_[y][x] = grid_[x][max - y - 1];
                return result;
            }

        private:
            Kind kind_;
            uint8_t grid_[4][4];
        }; 

        uint8_t grid(int x, int y) {
            // return the unattainable 0xff color which will stop rotations or falls when out of bounds
            if (x < 0 || y < 0 || x >= PLAY_WIDTH || y >= PLAY_HEIGHT)
                return 0xff; 
            return grid_[y * PLAY_WIDTH + x];
        }

        void setGrid(int x, int y, uint8_t color) {
            ASSERT(x >= 0 && y >= 0 && x < PLAY_WIDTH && y < PLAY_HEIGHT);
            grid_[y * PLAY_WIDTH + x] = color;
        }

        /** Validates the tetromino's position wrt the board.
         */
        bool validate(Tetromino const & t, int tx, int ty) {
            for (int x = 0; x < 4; ++x)
                for (int y = 0; y < 4; ++y)
                    if (t.grid(x,y) != 0 && grid(tx + x, ty + y) != 0)
                        return false;
            return true;
        }

        /** Spawns next tetromino. 
         */
        void spawn() {
            cur_ = next_;
            next_.randomize();
            x_ = 3;
            if (cur_.kind() == Tetromino::Kind::Beam || cur_.kind() == Tetromino::Kind::Square)
                y_ = -1;
            else 
                y_ = 0;
            // don't allow the next tile to go immediately down
            allowDown_ = false;
        }

        /** Rotates the current tetromino, if possible.
         */
        void rotate() {
            Tetromino tr = cur_.rotate();
            if (validate(tr, x_, y_))
                cur_ = tr;
            // TODO else rumble bad
            //else
            //   rumble()
        }

        void addToGrid(Tetromino const & t, int x, int y) {
            for (int ty = 0; ty < 4; ++ty)
                for (int tx = 0; tx < 4; ++tx) {
                    uint8_t c = t.grid(tx, ty);
                    if (c != 0)
                        setGrid(x + tx, y + ty, c);
                }
        }

        /** Checks row compacting when the tile lands. 
         */
        void compactRows(int startRow) {
            for (int y = startRow; y < startRow + 4; ++y) {
                bool compact = true;
                for (int x = 0; x < PLAY_WIDTH; ++x) {
                    uint8_t gc = grid(x, y);
                    if (gc == 0 || gc == 0xff) {
                        compact = false;
                        break;
                    }
                }
                if (compact) {
                    for (int yy = y; yy > 1; --yy)
                        for (int x = 0; x < PLAY_WIDTH; ++x)
                            setGrid(x, yy, grid(x, yy - 1));
                    for (int x = 0; x < PLAY_WIDTH; ++x)
                        setGrid(x, 0, 0);
                }
            }
        }

        /** Draws single tile at given location (absolute coordinates) 
         */
        void drawTile(int x, int y, uint8_t color) {
            if (color <= NUM_COLORS) {
                g_.fill(palette_[color], Rect::XYWH(x, y, 9, 9));
            } else {
                // TODO effects, etc
            }
        }

        void drawTetromino(int x, int y, Tetromino const & t) {
            int max = Tetromino::gridSize(t.kind());
            for (int ty = 0; ty < max; ++ty)
                for (int tx = 0; tx < max; ++tx) {
                    uint8_t c = t.grid(tx, ty);
                    if (c != 0)
                        drawTile(x + tx * 10, y + ty * 10, c);
                }
        }

        void resetGame() {
            speed_ = 60;
            for (unsigned i = 0; i < PLAY_WIDTH * PLAY_HEIGHT; ++i)
                grid_[i] = 0;
            next_.randomize();
            spawn();
            countdown_ = speed_;
        }

        /** The play area.  
         
            Stored value indicates color, 0 for black, which means empty. 
         */
        uint8_t grid_[PLAY_WIDTH * PLAY_HEIGHT]; 

        Tetromino cur_;
        Coord x_;
        Coord y_;
        Tetromino next_;
        uint32_t speed_ = 60;
        uint32_t countdown_;
        uint32_t score_ = 0;
        bool allowDown_ = true;

        /** Default grids of all 7 types. 
         */
        static constexpr uint8_t tetrominos_[][4][4] {
            {
                {0,0,0,0},
                {1,1,1,1},
                {0,0,0,0},
                {0,0,0,0},
            },
            {
                {0,0,0,0},
                {0,1,1,0},
                {0,1,1,0},
                {0,0,0,0},
            },
            {
                {0,1,0,0},
                {1,1,1,0},
                {0,0,0,0},
                {0,0,0,0},
            },
            {
                {1,0,0,0},
                {1,1,1,0},
                {0,0,0,0},
                {0,0,0,0},
            },
            {
                {0,0,1,0},
                {1,1,1,0},
                {0,0,0,0},
                {0,0,0,0},
            },
            {
                {1,1,0,0},
                {0,1,1,0},
                {0,0,0,0},
                {0,0,0,0},
            },
            {
                {0,1,1,0},
                {1,1,0,0},
                {0,0,0,0},
                {0,0,0,0},
            },
        };

        static constexpr ColorRGB palette_[] = {
            color::Black,
            color::Red, 
            color::Green,
            color::Yellow,
            color::Blue,
            color::Violet, 
            color::Cyan,
        };

    }; // rckid::Tetris


} // namespace rckid
