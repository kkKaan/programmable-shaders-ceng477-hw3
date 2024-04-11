# Bunny Run

## Introduction

Bunny Run is an OpenGL-based game developed for the Introduction to Computer Graphics course at Middle East Technical University. In this game, players control a bunny hopping in the horizontal direction to capture yellow checkpoints for points while avoiding red checkpoints. The game's challenge increases as the bunny's speed accelerates, demanding quick reflexes to achieve a high score.

## Table of Contents

- [Installation](#installation)
- [Gameplay](#gameplay)
- [Features](#features)
- [Dependencies](#dependencies)
- [Building the Game](#building-the-game)
- [Controls](#controls)
- [Contributors](#contributors)

## Installation

To play Bunny Run, clone the repository to your local machine:
\```bash
git clone <repository-url>
\```
Ensure you have the required dependencies installed (see Dependencies section).

## Gameplay

Bunny Run features an endlessly hopping bunny that players guide to capture yellow checkpoints for points while avoiding red checkpoints, which end the game. The bunny's speed increases with time, adding to the game's difficulty.

## Features

- **Dynamic Speed Increase:** The bunny's speed gradually increases, making each game progressively more challenging.
- **Hopping Animation:** A linear up-and-down motion that becomes quicker as the game progresses.
- **Checkpoint System:** Yellow checkpoints increase the score by 1000 points, while red checkpoints end the game.
- **Score System:** Points are awarded based on the distance covered and yellow checkpoints captured.

## Dependencies

Bunny Run requires the following libraries:
- OpenGL

## Building the Game

A Makefile is provided for building the game. Navigate to the game's directory and run:

\```bash
make
./main
\```

Ensure the build process is performed on a system compatible with the dependencies, such as Inek machines for METU students.

## Controls

- **A/D:** Move the bunny left/right.
- **R:** Restart the game.

## Contributors

- Kaan Karaçanta
- Taha Emir Gökçegöz
