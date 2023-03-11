#include "lib/thread_factory.h"

/****************************************************************************
 * Copyright (C) 2019 by Muhammed Bera KOÇ and Salih ÖZYURT                 *
 *                                                                          *
 * This file is part of IO Multithreading.                                  *
 *                                                                          *
 *   IO Multithreading is free software: you can redistribute it and/or     *
 *   modify it under the terms of the GNU Lesser General Public License     *
 *   as published by the Free Software Foundation, either version 3 of      *
 *   the License, or (at your option) any later version.                    *
 *                                                                          *
 *   IO Multithreading is distributed in the hope that it will be useful,   *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Lesser General Public License for more details.                    *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with IO Multithreading.                                  *
 *   If not, see <http://www.gnu.org/licenses/>.                            *
 ****************************************************************************/

/**
 * @author Salih ÖZYURT
 * @author Muhammed Bera KOÇ
 * @file main.c
 * @date 29 December 2019
 * @brief Contains main function

/**
 * @brief Invokes __init procedure to start the program
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char *argv[]) {
    __init((String *) argv, argc);
    puts("Program finished.");
    return EXIT_SUCCESS;
}
