См. [видео](https://youtu.be/i0UWrNF0Lmw).


----
## Условия задачи

In order to visualize a 3d sphere dataset in real-time, a CPU only solution should be implemented. Each sphere in the dataset is defined by position, radius and colour. A file containing this dataset should be provided with the test. A viewer that meets the following definition should be created:

* The image should be rendered to a 1024x1024@32bit buffer. 
* It can be assumed that the dataset is static and cannot be changed in run-time. 
* The spheres should be rendered using perspective projection.
* The spheres should be rendered using some HSR (hidden surface removal) algorithm.
* Simple shading (diffuse and specular) from a directional light source is sufficient. There’s no need for shadows.
* The spheres should spin in front of the camera. 
* Only the given sphere dataset in its entirety should be used for the assignment.

A framework with the initial implementation is provided with the test. The viewer should be created on top of that framework following those guidelines:

* Add the missing viewer functionality
* Make sure that the dataset is rendered efficiently
* Aim for performance, you’ll be measured both by the visual output and speed results.
* Use multiple threads (you may use Windows 7 thread pools or Vista, for example, don’t worry about portability or backward compatibility)
* Inner loop performance optimization (low level optimization, SSE, etc..)
* Stable results when the elements touch the view borders
* Explain why you made certain design decisions
* Comment the code to the degree that you feel is necessary
* Write clean and readable code. (use tab size 2)
* Implement the system with the appropriate data structures. High level libraries such as STL or Boost can be used.

As an option, a good looking (free style) version of the viewer can be provided as well. In this version only the rendering quality and visual output will be measured. If you do choose to do it, please make sure that you’ll are able to provide the source code and the executable for both of those versions.

Expected environment: Windows, you can assume PC Pentium4 (or similar AMD), C++




----
## Что сделано

1. Оптимизировал алгоритм сокрытия поверхностей. Добавил Z-буффер, чтобы избежать лишних расчётов при отрисовке.
2. Оптимизировал отрисовку круга. Создал простой адаптитвный пул, который запоминает построенные ранее окружности. См. CFrameBuffer::RenderSphere2(). Рисую круг только когда он попадает в фрейм.
3. Оптимизировал освещение по Фонгу. Вынес его в отдельный класс PhongShading. Если бы не условие задачи (CPU only), реализовал бы шейдерами.
4. Добавил псевдообъём для каждой точки круга. См. fScreenZ3D.
5. Заменил передачу множества параметров в рендер одной структурой FrameRenderElement.
6. Преобразовал структуру для хранения сферы SSphereElement для ускорения её сортировки. Выровнял её по 32 байтам.
7. Оптимизировал Vec3. Заменил double на float. Вынес класс в отдельный файл.
8. Заменил Vec3 на вектор с SIMD. Спасибо [Thomas Pelletier](https://github.com/pelletier/vector3). Оценить ускорение можно переопределяя typedef vec_t.
9. В Release конфигурации VS2019 включил: быстрый float и без float-exception, SIMD2, статическую линковку вместо DLL, максимальную оптимизацию по скорости.
10. Добавил const для методов и переменных.
11. Заменил defin'ы на static const.
12. Заменил CompareSpheresFunc() лямбдой.
13. Для доступа к началу списка элементов использовал std::data(list) вместо &list[0].
14. Для облегчения поддержки кода, ввёл новые типы данных. Например, frameBuffer_t.
15. Ввёл новый класс Shading. Используется для инкапсуляции разных типов освещений. См. DirectShading, PhongShading.
16. Добавил вращение по стрелкам клавиатуры.
17. Распараллелил расчёт глубины расположения сферы на сцене, сортировку по глубине, рендер сфер - см. CSphereData::Render(); а также рендер каждого пикселя сферы - см. CFrameBuffer::RenderSphere2().




----
## Что ещё можно сделать

1. Вращение сцены мышью. Пример моего [давнего проекта](https://github.com/signmotion/heightmap).
2. Инициализировать переменные в конструкторе прямо, через двоеточие.
3. Обязательные конструкторы и операторы для классов. Слово deleted.
stdafx для ускорения компиляции.
4. Отдельный файл / класс конфигурации, куда вынести все константы, включая глобальное освещение Light.
5. Больше всего времени занимает отрисовка сферы. Можно попробовать алгоритм Брезенхема вместо брутфорсного рисования круга.
6. Отрисовывать сцену Coverage- и Surface-буферами, BSP (Binary Space Partitioning) или даже рейкастом, благо, NVIDIA предоставляет поддержку RTX Ray Tracing.
7. Более продвинутое освещение. Например, Блин-Фонга.
8. Заменить std::pow() на [fastPow()](https://martin.ankerl.com/2012/01/25/optimized-approximative-pow-in-c-and-cpp).
9. Переписать загрузку данных через std::filesystem было бы полезно для будущего библиотеки.
10. Ограничить FPS с помощью std::this_thread::sleep_for(), уменьшив тем самым нагрузку на процессор и, если бы это была игра-шутер, так уравняем шансы игроков с компьютерами разной мощности. Сейчас я просто лишний раз не запускаю ренедр, если сцена осталась прежней. См. CFrameBuffer::m_wi_last.
11. Венгерская нотация не актуальна для современных IDE. Но, конечно же, всё упирается в гайдлайн проекта.
12. Структура проекта. Если SphereDataViewer позиционируется как библиотека, её структуру необходимо поменять. Как минимум, внутри проекта создать папку include/SphereDataViewer и поместить в неё H- и CPP-файлы, а сами классы обернуть в namespace. CMake-файл також стане у нагодi.
13. Классы разнести по отдельным файлам.
14. CFrameBuffer преобразовать в шаблон. Тогда вместо std::vector используем std::array. Это будет быстрее, ведь память в стеке горячее, чем в куче.
15. Могут возникнуть вопросы, имеет ли смысл использовать структуры массивов вместо массивов структур для SSphereElement и/или перемежывать массивы. Для современных компиляторов это не имеет смысла: он знает больше трюков, чем программист - с этим надо смириться) Да и поддерживать подобный "оптимизированный код" может превратиться для команды в локальный ад.
