#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

// ========== Шифр Нигилистов (6×6, русский + пробел, точка, запятая) ==========
void nihilist_buildTable(const wstring& keyword,
                         wchar_t table[6][6],
                         map<wchar_t, pair<int,int>>& charToPos);

vector<int> nihilist_encrypt(const wstring& text,
                              const wstring& gammaKey,
                              const wchar_t table[6][6],
                              const map<wchar_t, pair<int,int>>& pos);

wstring nihilist_decrypt(const vector<int>& ciphertext,
                          const wstring& gammaKey,
                          const wchar_t table[6][6],
                          const map<wchar_t, pair<int,int>>& pos);

// ========== Шифр Плейфера (русский 4×8 / английский 5×5) ==========
void playfair_buildMatrix(const wstring& keyword,
                          bool russian,
                          vector<vector<wchar_t>>& matrix,
                          map<wchar_t, pair<int,int>>& charPos);

wstring playfair_encrypt(const wstring& plaintext,
                          bool russian,
                          const vector<vector<wchar_t>>& matrix,
                          const map<wchar_t, pair<int,int>>& pos);

wstring playfair_decrypt(const wstring& ciphertext,
                          bool russian,
                          const vector<vector<wchar_t>>& matrix,
                          const map<wchar_t, pair<int,int>>& pos);