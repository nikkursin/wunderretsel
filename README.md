# Wunderrätsel

**Nature-motivated way to play with words**

Wunderrätsel is a mobile-first crossword puzzle application built with **Qt / QML and Felgo**.  
The main idea of the app is to combine vocabulary learning with a simple and satisfying visual reward system.

Instead of solving static levels, the user interacts with dynamically generated puzzles that reveal parts of an image. Each completed puzzle unlocks a full image, which is then stored in the gallery.

---

## Concept

The application follows a simple loop:

Solve → Reveal → Collect → Repeat

- The user solves crossword-style puzzles  
- Each correct word reveals a part of an image  
- Completing the puzzle unlocks the full image  
- Unlocked images are stored in the Gallery  

This creates a continuous motivation cycle based on progress and collection.

---

## Main Features

- Dynamic crossword puzzle generation  
- No predefined levels  
- Difficulty based on language level (A1–C2)  
- Visual reward system (image reveal)  
- Gallery of unlocked images  
- Local data storage (no backend required)  
- Custom UI built fully with QML  

---

## Screens

The application consists of several main screens:

### Onboarding
Initial setup of user preferences:
- Language level selection  
- Content preference selection  

### Home
Main navigation hub:
- Start a new puzzle  
- Open gallery  
- Go to settings  

### Play
Core gameplay screen:
- Crossword puzzle interaction  
- Image reveal logic  
- Puzzle completion state  

### Gallery
Displays user progress:
- Unlocked images  
- Locked placeholders  
- Progress overview  

### Settings
Allows modifying preferences:
- Language level  
- Content type  

---

## Architecture

The project follows a **Qt-oriented MVVM approach**:

- **QML** → UI layer (screens and components)  
- **C++ Managers** → application logic and state  
- **Models** → data structures  
- **Storage** → local persistence  

All logic is separated from UI, making the project scalable and maintainable.

---

## Data Handling

The application is fully **local-first**:

### Static Data
Stored in JSON files:
- words  
- levels  
- image metadata  

### User Data
Stored locally using QSettings or Felgo Storage:
- preferences  
- progress  
- unlocked images  

---

## UI Approach

- Built using pure QML (no default Felgo UI components)  
- Custom reusable components  
- Minimalistic and consistent design  
- Mobile-first layout  
- Clear interaction and selection states  

---

## Purpose

The goal of the project is to demonstrate:

- Clean architecture with Qt / Felgo  
- Separation of UI and logic  
- Dynamic content generation  
- Scalable design for mobile applications  

---

## Status

The project includes:
- Core navigation structure  
- UI for all main screens  
- Basic architecture setup  

Further improvements can include:
- more advanced puzzle generation  
- additional content  
- performance optimizations  

---

## Requirements

- Qt 6  
- Felgo SDK  
- Qt Creator  

---

## Running the Project

1. Open the project in Qt Creator  
2. Configure your build kit  
3. Run the application on emulator or device  

---

## Notes

This project is designed as a **learning and demonstration application** focused on architecture, UI design, and interaction patterns using Qt and Felgo.
