Запуск:
`./build/program2/program2 127.0.0.1 5000`
`./build/program1/program1 127.0.0.1 5000`

Тестовые примеры:
- Вводим в program1 "1234", получаем в program2 "Not accepted: sum '4' does not satisfy the criteria."
- Вводим в program1 "9999999999999911", получаем в program2 "Accepted: sum 128 is longer than 2 characters and divisible by 32!"

Пример консоли сервера:
```
Program 2 is listening on port 5000. Waiting for program 1...
Client 127.0.0.1:49788 connected.
Not accepted: sum '4' does not satisfy the criteria.
Accepted: sum 128 is longer than 2 characters and divisible by 32!
Client 127.0.0.1:49788 disconnected.
Client 127.0.0.1:43800 connected.
Not accepted: sum '1' does not satisfy the criteria.
Client 127.0.0.1:43800 disconnected.
```

Решение:

Клиент (program1/)
- Два потока: первый принимает и проверяет ввод пользователя, второй отправляет сумму и выводит в консоль
- Общий буфер - SharedBuffer из common/, сделан через мьютекс и conditional variable
- Пытается повторно подключитбся на потере соединения

Сервер (program2/)
- Принимает данные от program1 и проверяет через библиотечные функции
- Обрабатывает клиентов на одном потоке
- При отключении клиента(ов) ожидает повторного подключения

Библиотека (libstring/)
- Динамическая библиотека с функциями из ТЗ

Common
- FdWrapped: RAII обертка над fd. Поддерживает безопасное перемещение владения.
- parse/: утилиты для парсинга строк

Заметки
- Программы сделаны как *_core чтобы их можно было подключать как библиотеку в GTest
- Утилиты и common файлы вынесены в статическую библиотеку common/
- TcpServer и TcpClient имеют небольшую привязку к прикладным задачам program1, program2, поэтому не вынесены в common/