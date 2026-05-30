//=============================================================================
// randomness.h - Simple header for all your random needs
//=============================================================================
// Provides 50+ predefined randomness ranges and utilities for game development.
// Include this header and use the functions directly in if statements.
//=============================================================================

#pragma once

#include <random>
#include <vector>
#include <chrono>

//=============================================================================
// Global random engine (seeded once)
//=============================================================================
namespace Random {
    inline std::mt19937& generator() {
        static std::mt19937 gen(static_cast<unsigned int>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        ));
        return gen;
    }

    //=========================================================================
    // Core functions
    //=========================================================================
    inline int Int(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(generator());
    }

    inline float Float(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(generator());
    }

    inline bool Bool(float probability = 0.5f) {
        return Float(0.0f, 1.0f) < probability;
    }

    template<typename T>
    inline T& Element(std::vector<T>& vec) {
        return vec[Int(0, static_cast<int>(vec.size()) - 1)];
    }

    template<typename T>
    inline const T& Element(const std::vector<T>& vec) {
        return vec[Int(0, static_cast<int>(vec.size()) - 1)];
    }

    //=========================================================================
    // Predefined integer ranges (50+ of them)
    //=========================================================================

    // Basic small ranges
    inline int OneToTwo()     { return Int(1, 2); }
    inline int OneToThree()   { return Int(1, 3); }
    inline int OneToFour()    { return Int(1, 4); }
    inline int OneToFive()    { return Int(1, 5); }
    inline int OneToSix()     { return Int(1, 6); }
    inline int OneToEight()   { return Int(1, 8); }
    inline int OneToTen()     { return Int(1, 10); }
    inline int OneToTwelve()  { return Int(1, 12); }
    inline int OneToTwenty()  { return Int(1, 20); }
    inline int OneToFifty()   { return Int(1, 50); }
    inline int OneToHundred() { return Int(1, 100); }

    // Zero-based
    inline int M_To_N(int x,int y)    { return Int(x, y); }
    inline int ZeroToTwenty() { return Int(0, 20); }

    // Symmetric ranges (negative to positive)
    inline int MinusFiveToFive()      { return Int(-5, 5); }
    inline int MinusTenToTen()        { return Int(-10, 10); }
    inline int MinusTwentyToTwenty()  { return Int(-20, 20); }

    // Dice (classic)
    inline int D4()  { return Int(1, 4); }
    inline int D6()  { return Int(1, 6); }
    inline int D8()  { return Int(1, 8); }
    inline int D10() { return Int(1, 10); }
    inline int D12() { return Int(1, 12); }
    inline int D20() { return Int(1, 20); }
    inline int D100(){ return Int(1, 100); }

    // Percentage (0-100 inclusive)
    inline int Percent() { return Int(0, 100); }

    // Multiples of specific steps
    inline int MultipleOfFive(int min = 1, int max = 100) {
        int steps = (max - min) / 5;
        return min + 5 * Int(0, steps);
    }
    inline int MultipleOfTen(int min = 1, int max = 100) {
        int steps = (max - min) / 10;
        return min + 10 * Int(0, steps);
    }
    inline int MultipleOfTwenty(int min = 1, int max = 100) {
        int steps = (max - min) / 20;
        return min + 20 * Int(0, steps);
    }

    // Predefined multiple-of-5 sets
    inline int OneToFiftyByFive()   { return 5 * Int(1, 10); }   // 5,10,...,50
    inline int OneToHundredByFive() { return 5 * Int(1, 20); }   // 5,10,...,100
    inline int ZeroToHundredByFive(){ return 5 * Int(0, 20); }   // 0,5,...,100

    // Specific sets
    inline int Set_1_5_10_15() {
        const int choices[] = {1,5,10,15};
        return choices[Int(0, 3)];
    }
    inline int Set_2_4_6_8_10() {
        const int choices[] = {2,4,6,8,10};
        return choices[Int(0, 4)];
    }
    inline int Set_25_50_75_100() {
        const int choices[] = {25,50,75,100};
        return choices[Int(0, 3)];
    }
    inline int Set_10_20_30_40_50() {
        const int choices[] = {10,20,30,40,50};
        return choices[Int(0, 4)];
    }
    inline int Set_33_66_100() {
        const int choices[] = {33,66,100};
        return choices[Int(0, 2)];
    }

    // More ranges (1-3, 1-4, 2-5, 3-7 etc)
    inline int TwoToFive()   { return Int(2, 5); }
    inline int ThreeToSeven(){ return Int(3, 7); }
    inline int FiveToTen()   { return Int(5, 10); }
    inline int TenToTwenty() { return Int(10, 20); }

    // Special game design ranges
    inline int LowDamage()    { return Int(1, 3); }      // weak attacks
    inline int MediumDamage() { return Int(4, 8); }      // normal hits
    inline int HighDamage()   { return Int(9, 15); }     // critical
    inline int HealingSmall() { return Int(2, 5); }
    inline int HealingMedium(){ return Int(6, 12); }
    inline int HealingLarge() { return Int(15, 25); }

    // Random with exclusion? Not included for simplicity.

    //=========================================================================
    // Float ranges
    //=========================================================================
    inline float ZeroToOne()        { return Float(0.0f, 1.0f); }
    inline float M_To_N_Float(float x,float y){ return Float(0.0f, 5.0f); }
    inline float ZeroToTenf()        { return Float(0.0f, 10.0f); }
    inline float MinusOneToOne()    { return Float(-1.0f, 1.0f); }
    inline float AngleDegrees()     { return Float(0.0f, 360.0f); }
    inline float AngleRadians()     { return Float(0.0f, 2.0f * 3.14159265f); }

    //=========================================================================
    // Boolean / chance
    //=========================================================================
    inline bool OneInTwo()   { return Int(1, 2) == 1; }
    inline bool OneInThree() { return Int(1, 3) == 1; }
    inline bool OneInFour()  { return Int(1, 4) == 1; }
    inline bool OneInFive()  { return Int(1, 5) == 1; }
    inline bool OneInSix()   { return Int(1, 6) == 1; }
    inline bool OneInTen()   { return Int(1, 10) == 1; }
    inline bool OneInTwenty(){ return Int(1, 20) == 1; }
    inline bool OneInHundred(){return Int(1, 100) == 1; }

    // Chance with percentage (0-100)
    inline bool ChancePercent(int percent) {
        return Int(1, 100) <= percent;
    }

    //=========================================================================
    // Example usage in if statements:
    //
    // if (Random::OneInTen()) { /* rare event */ }
    // if (Random::Int(1,5) == 3) { /* do something */ }
    // int damage = Random::D20() + 5;
    // if (Random::Set_1_5_10_15() == 10) { /* special case */ }
    //=========================================================================
} // namespace Random