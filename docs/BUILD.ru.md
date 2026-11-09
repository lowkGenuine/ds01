# Сборка DEESS-01

[English](BUILD.en.md) · [Описание](../README.ru.md)

Нужны CMake 3.22 или новее, компилятор C++17 и исходники JUCE 7.0.12. Готовый релиз — Windows x64 VST3. Сборки для macOS и Linux не проверялись.

## Настроить и собрать

Выполни команды из корня репозитория, подставив путь к JUCE:

```powershell
cmake -S . -B build -DJUCE_PATH="C:/path/to/JUCE-7.0.12" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Бандл VST3 появится в `build/Deess01_artefacts/Release/VST3/DEESS-01.vst3`. Устанавливай его целиком по [пользовательской инструкции](../README.ru.md#скачать-и-установить).

Для MinGW/LLVM-MinGW добавь папку `bin` компилятора в PATH на время настройки и сборки: JUCE запускает вспомогательную программу для манифеста. Сборка MinGW статически подключает библиотеку времени выполнения C++.

## Проверки

CTest запускает независимый тест DSP. При полной сборке также создаются:

- `Deess01HostCheck`: первым аргументом передай полный путь к бандлу `.vst3`.
- `Deess01Preview`: передай пути для PNG основного экрана и Details.

Расположение программ зависит от генератора: Visual Studio обычно использует `build/Release`, MinGW — `build`.

Чтобы собрать только тест DSP без JUCE:

```powershell
cmake -S . -B build-dsp -DDEESS01_BUILD_PLUGIN=OFF
cmake --build build-dsp --config Release
ctest --test-dir build-dsp -C Release --output-on-failure
```

Что проверено в готовом бинарнике 0.2.0, описано в [результатах проверки](VALIDATION.ru.md). Условия JUCE приведены в [уведомлении о зависимости](../ThirdParty/JUCE-LICENSE.md). Отдельной лицензии для собственного кода проекта пока нет.
