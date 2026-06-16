# Fate 🔮

A program providing alternative, spiritually (in)accurate diagnostics for Linux
PIDs.

## What and especially why?

This program outputs process information based on the supplied PID in a
strange/mystic/zodiac fashion, as a homage to the Unix command `fortune`,
originally released in 1979.

## Installation

To install from source, clone this repository and run
```bash
cmake .
sudo make
```

## Usage

Getting information about a single PID is simple:
```bash
fate <pid_value>
```
