# Shower Niagara FX

![Shower Niagara FX](Media/ShowerNiagara.gif)

---

## 🇷🇺 Русский

**Shower Niagara FX** — самодостаточный эффект душа для Unreal Engine 5.8: латунный душ из примитивов, круглые капли воды, брызги о пол, пар и зацикленный звук воды. Всё управляется из Details или прямо из блюпринта.

### Возможности

- 💧 Вода — круглые капли (Niagara), дуга струи + брызги у пола
- 🌫️ Пар и туман — отдельные системы с настройкой плотности, размера, радиуса, цвета и прозрачности
- 🔊 Зацикленный звук воды с громкостью и радиусом слышимости
- 🎛️ Материалы-инстансы: цвет и прозрачность воды/пара меняются в один клик
- 🧩 C++ актор `ShowerActor` + блюпринт-наследник `BP_Shower` с нодами, ивентами и диспатчерами
- 📦 Самодостаточен: свои меши, материалы, звук; из движка нужен только плагин Niagara

### Установка

1. Скопируйте папку `Plugins/ShowerFX` в папку `Plugins/` вашего проекта.
2. Скопируйте папку `Content/VFX/Shower` (и при желании демо-уровень `Content/VFX/L_ShowerDemo.umap`) в `Content/` вашего проекта.
3. Запустите проект:
   - **UE 5.8** — готовые бинарники уже лежат в `Plugins/ShowerFX/Binaries/Win64`, плагин подхватится сам;
   - **UE 5.7 / 5.2** — скопируйте содержимое `Binaries/UE_5.7/Win64` (или `Binaries/UE_5.2/Win64`) поверх `Plugins/ShowerFX/Binaries/Win64`;
   - **C++-проект** — редактор предложит пересобрать модуль из исходников (VS 2022).
4. Перетащите `BP_Shower` из `Content/VFX/Shower` в свой уровень.

> Демо-уровень `L_ShowerDemo` показывает душ в «комнате» из плитки — можно использовать как референс освещения и расстановки.

### Быстрая настройка (Details актора)

| Группа | Параметры |
|---|---|
| **Water** | Flow Rate (плотность струи), Stream Speed, Stream Spread (разлёт), Drop Size Min/Max, Mist Rate, Water Offset, Water Visible |
| **Splash** | Splash Rate, Splash Speed, Splash Size Min/Max, Splash Offset, Splash Visible |
| **Steam** | Steam Rate, Steam Radius, Steam Size Min/Max, Steam Color (альфа = прозрачность), Steam Offset, Steam Visible |
| **Sound** | Water Sound, Sound Volume, Sound Attenuation Radius (радиус слышимости), Sound On |
| **Materials** | Water Material, Steam Material — любые материалы/инстансы |
| **Assets** | Системы Niagara, меши трубы и лейки, латунный материал |

### Управление из блюпринта

На `BP_Shower` доступны ноды (категория **ShowerFX**):

- `Start Shower` / `Stop Shower` — включить/выключить всё разом
- `Set Water Visible`, `Set Splash Visible`, `Set Steam Visible` — показать/скрыть часть эффекта
- `Set Sound On` — звук
- `Set Flow Rate`, `Set Steam Rate` — плотность на лету

Диспетчеры (Event Dispatchers): `On Water Changed`, `On Splash Changed`, `On Steam Changed`, `On Sound Changed` — сообщают миру о переключениях.

Ивенты для реализации графиком в `BP_Shower`: `Event On Shower Started`, `Event On Shower Stopped`.

Пример: мировое событие → ссылка на `BP_Shower` → `Start Shower` → душ включился, а подписчики `On Water Changed` получили `true`.

### Состав

```
Content/VFX/Shower/       BP_Shower, системы Niagara (NS_ShowerWater/Splash/Steam),
                          материалы (M_Brass, M_WaterDrop[+Inst], M_Steam[+Inst],
                          M_TileWall, M_FloorStone), меши, звук
Content/VFX/              L_ShowerDemo — демо-уровень
Plugins/ShowerFX/         C++ плагин (актор ShowerActor + редакторская утилита)
Media/                    GIF-превью
```

### Требования и версии UE

| Версия UE | Плагин (бинарники) | Контент (`BP_Shower`, системы Niagara) |
|---|---|---|
| **5.8** | ✅ `Binaries/UE_5.8` (уже в `Binaries/Win64`) | ✅ полный |
| **5.7** | ✅ `Binaries/UE_5.7` | ⚠️ uasset-ы сохранены в 5.8 — Niagara-системы и `BP_Shower` пересоздайте под 5.7; актор `Shower Actor`, геометрия и звук работают |
| **5.2** | ✅ `Binaries/UE_5.2` | ⚠️ аналогично 5.7 |

Исходники плагина компилируются на 5.2–5.8 (автолинковка Niagara-параметров — только 5.8+). На версиях младше 5.8 задайте связи `User.*` вручную в UI Niagara или управляйте системами напрямую.

- Плагин **Niagara** (включён по умолчанию)

### Примечание о звуке

`S_WaterLoop` — копия лупа из набора **MedFantasy SFX Bundle**, импортированного в проект-источник. Перед публикацией своих проектов проверьте лицензию этого набора или замените звук на собственный (поле `Water Sound`).

---

## 🇬🇧 English

**Shower Niagara FX** is a self-contained shower effect for Unreal Engine 5.8: a brass shower built from primitives, round water droplets, floor splash, steam and a looping water sound. Everything is tweakable from the Details panel or directly from Blueprint.

### Features

- 💧 Water — round Niagara droplets, arcing stream + floor splash
- 🌫️ Steam & mist — separate systems with density, size, radius, color and opacity controls
- 🔊 Looping water sound with volume and audible radius
- 🎛️ Material instances: water/steam color and opacity in one click
- 🧩 C++ `ShowerActor` + `BP_Shower` Blueprint child with functions, events and dispatchers
- 📦 Self-contained: custom meshes, materials and sound; only the stock Niagara plugin is required

### Installation

1. Copy the `Plugins/ShowerFX` folder into your project's `Plugins/` folder.
2. Copy `Content/VFX/Shower` (and optionally the demo level `Content/VFX/L_ShowerDemo.umap`) into your project's `Content/` folder.
3. Open the project:
   - **UE 5.8** — prebuilt binaries already sit in `Plugins/ShowerFX/Binaries/Win64`, the plugin loads automatically;
   - **UE 5.7 / 5.2** — copy the contents of `Binaries/UE_5.7/Win64` (or `Binaries/UE_5.2/Win64`) over `Plugins/ShowerFX/Binaries/Win64`;
   - **C++ project** — the editor will offer to rebuild the module from source (VS 2022).
4. Drag `BP_Shower` from `Content/VFX/Shower` into your level.

> The `L_ShowerDemo` level shows the shower inside a tiled "room" — use it as a lighting/layout reference.

### Quick tuning (actor Details)

| Group | Parameters |
|---|---|
| **Water** | Flow Rate, Stream Speed, Stream Spread, Drop Size Min/Max, Mist Rate, Water Offset, Water Visible |
| **Splash** | Splash Rate, Splash Speed, Splash Size Min/Max, Splash Offset, Splash Visible |
| **Steam** | Steam Rate, Steam Radius, Steam Size Min/Max, Steam Color (alpha = opacity), Steam Offset, Steam Visible |
| **Sound** | Water Sound, Sound Volume, Sound Attenuation Radius, Sound On |
| **Materials** | Water Material, Steam Material — any material or instance |
| **Assets** | Niagara systems, pipe/head meshes, brass material |

### Blueprint control

`BP_Shower` exposes these nodes (category **ShowerFX**):

- `Start Shower` / `Stop Shower` — toggle everything at once
- `Set Water Visible`, `Set Splash Visible`, `Set Steam Visible` — show/hide parts
- `Set Sound On` — sound
- `Set Flow Rate`, `Set Steam Rate` — live density

Event Dispatchers: `On Water Changed`, `On Splash Changed`, `On Steam Changed`, `On Sound Changed` — broadcast the current state.

Blueprint-implementable events on `BP_Shower`: `Event On Shower Started`, `Event On Shower Stopped`.

Example: a world event → `BP_Shower` reference → `Start Shower` → the shower turns on and every `On Water Changed` subscriber receives `true`.

### Layout

```
Content/VFX/Shower/       BP_Shower, Niagara systems (NS_ShowerWater/Splash/Steam),
                          materials (M_Brass, M_WaterDrop[+Inst], M_Steam[+Inst],
                          M_TileWall, M_FloorStone), meshes, sound
Content/VFX/              L_ShowerDemo — demo level
Plugins/ShowerFX/         C++ plugin (ShowerActor + editor utility module)
Media/                    GIF preview
```

### Requirements & UE versions

| UE version | Plugin (binaries) | Content (`BP_Shower`, Niagara systems) |
|---|---|---|
| **5.8** | ✅ `Binaries/UE_5.8` (already in `Binaries/Win64`) | ✅ full |
| **5.7** | ✅ `Binaries/UE_5.7` | ⚠️ uassets are saved in 5.8 — recreate the Niagara systems and `BP_Shower` for 5.7; the `Shower Actor`, geometry and sound work |
| **5.2** | ✅ `Binaries/UE_5.2` | ⚠️ same as 5.7 |

The plugin sources compile on 5.2–5.8 (automatic Niagara parameter linking is 5.8+ only). On engines older than 5.8 set the `User.*` bindings manually in the Niagara UI or drive the systems directly.

- **Niagara** plugin (enabled by default)

### Sound note

`S_WaterLoop` is a copy of a loop from the **MedFantasy SFX Bundle** imported into the source project. Check that bundle's license before redistributing, or replace the sound with your own via the `Water Sound` field.

---

## License / Лицензия

MIT — see [LICENSE](LICENSE).
