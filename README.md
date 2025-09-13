# SpectralAbyss Multiband-Saturation VST Plugin

<img width="500" alt="SpectralAbyssScreenshot" src="https://github.com/user-attachments/assets/0f55000b-6675-4339-9b2d-c05b0eaf80f4" />

**SpectralAbyss** is a multiband saturation audio plugin built with [JUCE](https://juce.com/).
It allows the distortion of three individual frequency bands using two different distortion algorithms.

## Features
* Three independent frequency bands
* Two distortion types (tanh and tape)
* Spectrum analyser
* Automatic makeup gain

## Installation
* Download the latest Release for your operating system from the [Release Page](https://github.com/pschaupp/spectral-abyss-multiband-saturation/releases/latest)
* Unzip the directory
* Move the unzipped directory into your VST3 Plugin Folder:
  ```bash
  # Windows
  C:\Program Files\Common Files\VST3
  
  # macOS
  ~/Library/Audio/Plug-Ins/VST3 # or
  /Library/Audio/Plug-Ins/VST3
  ```
* Note: On macOS you also need to remove the quarantine attribute that macOS adds to downloaded files:
  ```bash
  xattr -rd com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/SpectralAbyss-build-v001-macOS
  ```
