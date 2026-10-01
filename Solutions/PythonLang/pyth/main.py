import exotic_objs as ex

# ------ Односвязный список ------
print("Демонстрация работы односвязного списка:")

sll = ex.SinglyLinkedList()
print("Создаем список: " + str(sll))
sll.prepend(1)
print("Добавляем в начало: " + str(sll))
sll.append(2)
print("Добавляем в конец: " + str(sll))
sll.insert_after(1, 3)
print("Вставляем 3 после индекса 1: " + str(sll))
sll.insert_before(0, 1)
print("Вставляем 1 до индекса 0: " + str(sll))
sll.insert_after(2, 2)
print("Вставляем 2 после индекса 2: " + str(sll))

print("Поиск элементов:")
print("Индекс значения 2: ", sll.find(2))
s = ""
for x in sll.find_all(1):
	s += str(x)
print("Все индексы значения 1: " + s)
print("Проверка наличия 4: ", sll.contains(4))

print("Удаление элементов:")
sll.remove_at(2)
print("Удаление элемента с индекса 2: " + str(sll))
print("Удаление первого вхождения 1: " + str(sll))
print("Удаление первого элемента: " + str(sll))
print("Удаление последнего элемента: " + str(sll))

print("Получить инфу узла по индексу: ", sll.get(0))

print("Перегрузка операторов:")
print("Получение длины: ", len(sll))
print("Строковое представление: ", str(sll))
a = list(sll)
print("Сделать итерируемым: ", [str(x) for x in a])
print("Получить значение по индексу: ", sll[0])
sll[0] = 3
print("Изменить значение узла по индексу: ", str(sll))
print("Проверка на содержание значения: ", 2 in sll)
sll2 = ex.SinglyLinkedList()
sll2.append(2)
sll2.append(1)
print("Конкатенация списков: ", sll + sll2)
print("Умножение списка: ", sll * 3)

print("Обработка исключений.\nМетоды insert_after, insert_before, remove_first,\n remove_last, remove_at, remove_value, get.")
sll_e = ex.SinglyLinkedList()
try:
	sll.insert_after(10, 2)
except ex.IndexError as e:
	print(f"Ошибка IE: {e}")
try:
	sll.insert_before(-1, 1)
except ex.IndexError as e:
	print(f"Ошибка IE: {e}")
try:
	sll_e.remove_first()
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")
try:
	sll_e.remove_last()
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")
try:
	sll_e.remove_at(10)
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")
except ex.IndexError as e:
	print("Ошибка IE: {e}")
try:
	sll_e.remove_value(13) #пустой
	sll.remove_value(13) #непустой
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")
except ex.ValueNotFoundError as e2:
	print(f"Ошибка VNFE: {e2}")
try:
	sll.get(50)
except ex.IndexError as e:
	print(f"Ошибка IE: {e}")

# ------ Двусвязный список ------
print("Демонстрация работы двусвязного списка:")

dll = ex.DoublyLinkedList()
print("Создаем список: " + str(dll))
dll.prepend(1)
print("Добавляем в начало: ", str(dll))
dll.append(2)
print("Добавляем в конец: ", str(dll))
dll.insert_after(1, 3)
print("Вставляем 3 после индекса 1: " + str(dll))
dll.insert_before(0, 1)
print("Вставляем 1 до индекса 0: " + str(dll))
dll.insert_after(2, 2)
print("Вставляем 2 после индекса 2: " + str(dll))

print("Поиск элементов:")
print("Индекс значения 2: ", dll.find(2))
s = ""
for x in dll.find_all(1):
        s += str(x)
print("Все индексы значения 1: " + s)
print("Проверка наличия 4: ", dll.contains(4))

print("Удаление элементов:")
dll.remove_at(2)
print("Удаление элемента с индекса 2: " + str(dll))
print("Удаление первого вхождения 1: " + str(dll))
print("Удаление первого элемента: " + str(dll))
print("Удаление последнего элемента: " + str(dll))

print("Получить инфу узла по индексу: ", dll.get(0))

print("Перегрузка операторов:")
print("Получение длины: ", len(dll))
print("Строковое представление: ", str(dll))
a = list(dll)
print("Сделать итерируемым: ", [str(x) for x in a])
print("Получить значение по индексу: ", dll[0])
dll[0] = 3
print("Изменить значение узла по индексу: ", str(dll))
print("Проверка на содержание значения: ", 2 in dll)
dll2 = ex.SinglyLinkedList()
dll2.append(2)
dll2.append(1)
print("Конкатенация списков: ", dll + dll2)
print("Умножение списка: ", dll * 3)

print("Обработка исключений.\nМетоды insert_after, insert_before, remove_first,\n remove_last, remove_at, remove_value, get.")
dll_e = ex.DoublyLinkedList()

try:
	dll.insert_after(10, 2)
except ex.IndexError as e:
	print(f"Ошибка IE: {e}")
try:
	dll.insert_before(-1, 1)
except ex.IndexError as e:
	print(f"Ошибка IE: {e}")
try:
	dll_e.remove_first()
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")
try:
	dll_e.remove_last()
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")
try:
	dll_e.remove_at(10)
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")
except ex.IndexError as e:
	print(f"Ошибка IE: {e}")
try:
	dll_e.remove_value(13) #пустой
	dll.remove_value(13) #непустой
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")
except ValueNotFoundError as e2:
	print(f"Ошибка VNFE: {e2}")
try:
	dll.get(50)
except IndexError as e:
	print(f"Ошибка IE: {e}")

# --------- Стек ---------
print("Демонстрация работы стека:")
stc = ex.Stack()
stc.push(1)
print("Создаем стек + добавляем элемент 1 в стек(демонстрация верхнего элемента): ", stc.peek())
stc.push(2)
print("Добавляем элемент в стек: ", stc.peek())
print("Удалить последний элемент и показать его: ", stc.pop())
print("Показать верхний элемент стека: ", stc.peek())
print("Проверить на пустоту: ", stc.is_empty())
print("Получить размер стека: ", stc.size())
que_test = ex.Queue()
que_test.enqueue(1)
que_test.enqueue(2)
que_test.enqueue(3)
stacky = ex.Stack().from_queue(que_test)
print("Преобразовать очередь в стек: ", stacky.peek())

print("Обработка исключений.\nМетоды pop, peek")
stacy = ex.Stack()
try:
	stacy.pop()
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")
try:
	stacy.peek()
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")

# -------- Очередь --------
print("Демонстрация работы очереди:")
que = ex.Queue()
que.enqueue(1)
print("Создаем очередь(первый элемент): ", que.front())
que.enqueue(2)
print("Добавляем в очередь: ", que.front())
que.dequeue()
print("Удаление элемента: ", que.front())
print("Проверка на пустоту: ", que.is_empty())
print("Получить размер очереди: ", que.size())
stk_test = ex.Stack()
stk_test.push(1)
stk_test.push(2)
stk_test.push(3)
quuee = ex.Queue().from_stack(stk_test)
print("Преобразовать стек в очередь: ", quuee.front())

print("Обработка исключений.\nМетоды dequeue, front")
queick = ex.Queue()

try:
	queick.dequeue()
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")
try:
	queick.front()
except ex.EmptyStructureError as e:
	print(f"Ошибка ESE: {e}")

# ----- FIN ----- #
