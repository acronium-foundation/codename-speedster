/**
 * @file kernel.c
 * @brief Главный файл ядра - точка входа в систему
 */

#include "video/video.h"
#include "idt/idt.h"
#include "drivers/keyboard.h"
#include "drivers/pit.h"
#include "memory/memory.h"
#include "syscall/syscall.h"
#include "console.h"
#include "shell.h"
#include <stdarg.h>
#include <stdbool.h>
 
 void kernel_panic(const char* msg);
 
 /* Внешние символы для определения размера ядра */
 extern uint32_t _kernel_start;
 extern uint32_t _kernel_end;
 
 /**
  * @brief Точка входа в ядро операционной системы
  */
 void kmain(void) 
 {
    idt_init();         // Настройка таблицы прерываний
    keyboard_init();    // Инициализация драйвера клавиатуры
    pit_init();         // Инициализация системного таймера
    
    /* Инициализация менеджера памяти */
    pmm_init((uint32_t)&_kernel_end);
     
    /* Инициализация кучи ядра (начинаем после ядра) */
    uint32_t heap_start = align_up((uint32_t)&_kernel_end + 1024 * 1024, PAGE_SIZE); /* 1MB после ядра */
    uint32_t heap_size = 1024 * 1024; /* 1MB для кучи */
    heap_init(heap_start, heap_size);
    
    /* Инициализация подсистемы системных вызовов */
    syscall_init();

    clear_screen();
     
    /* Вывод информации о ядре */
    const char *kernel_name = "\nlibreacronium\n";
    const char *kernel_msg = "2024-2026 (c) Acronium Foundation\n";

     
    // Название ядра с цветовым оформлением
    print_string_color(kernel_name, COLOR_GREEN, COLOR_RED);
    // Информация о копирайте
    print_string(kernel_msg);
 
    /* Запуск тестов менеджера памяти */
    //run_memory_tests();
 
    /* Запуск тестов системного таймера */
    //run_timer_tests();
 
    /**
     * @brief Основной цикл ядра с временным псевдо-терминалом
     * 
     * @note Временный псевдо-терминал доступен ТОЛЬКО в режиме разработки (DevTest)
     *       и будет удален в финальной версии. В production-среде терминальный ввод/вывод
     *       будет доступен исключительно из userspace через системные вызовы.
     */
    while(true) {
        /* Временное приглашение командной строки */
        console_prompt();
        
        /**
         * @dev Временная функция чтения ввода (только для DevTest)
         * 
         * @note В production-реализации:
         * - Ввод будет обрабатываться через механизм прерываний
         * - Буферизация будет осуществляться в пространстве пользователя
         * - Доступ к терминалу будет через стандартные дескрипторы (stdin/stdout)
         */
        char* user_input = console_readline(128);
 
        if (user_input) {
            /* Обработка команды в shell */
            shell_execute(user_input);
 
           /* Освобождаем память, выделенную для ввода */
            kfree(user_input);
        } else {
            /* Если не удалось выделить память, ждем немного */
            pit_sleep_ms(100);
        }
         
       /* Используем hlt для экономии энергии, когда ядру нечего делать */
        /* В будущем здесь будет планировщик задач */
        __asm__ volatile("hlt");
    }
     
    /* Ядро никогда не должно достигать этой точки */
    __builtin_unreachable();
 }
 
 /**
  * @brief Аварийное завершение работы ядра (kernel panic)
  * @param msg Строка с причиной паники
  */
 void kernel_panic(const char* msg) {
     clear_screen();
     disable_cursor();
     print_string_color("KERNEL PANIC!\n", COLOR_WHITE, COLOR_RED);
     print_string_color(msg, COLOR_BLACK, COLOR_WHITE);
     while (1) {
         __asm__ volatile ("cli; hlt");
     }
 }
