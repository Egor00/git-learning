#лабораторная по питону номер 1
class StructureError(Exception):
	""" родительский класс ошибки
	класс, от которого наследуются все ошибки"""
	pass

class EmptyStructureError(StructureError):
	""" класс обработки ошибки пустой структуры
	Обрабатывает ошибки, связанные с пустыми структурами """
	def __init__(self, structure_name, message = "Structure is empty"):
		""" инициализатор ошибки
		Считывает имя структуры, в которой произошла ошибка
		и сообщение ошибки """
		self.structure_name = structure_name
		super().__init__(f"{message}: {structure_name}")

class IndexError(StructureError, IndexError):
	""" класс обработки ошибок индекса
	Обрабатывает ошибки невалидного индекса """
	def __init__(self, index, message = "Index out of range"):
		""" инициализатор ошибки
		Считывает невалидный индекс и сообщение ошибки """
		self.index = index
		super().__init__(f"{message}: {index}")

class ValueNotFoundError(StructureError):
	""" класс обработки ошибки ненайденного значения
	Обрабатывает ошибку в случае если заданного значения нет в списке """
	def __init__(self, value, message = "Value not found"):
		""" инициализатор ошибки
		Считывает значение и сообщение ошибки """
		self.value = value
		super().__init__(f"{message}: {value}")

class Node:
	""" Узел
	Задается узел. Методы:
	__init__ Инициализатор
	clear очистка данных узла
	__str__ строковое представление узла """
	def __init__(self, data = 0, next = None):
		""" Инициализатор узла
		Поля: data - число
		      next - ссылка на следующий узел """
		self.data = data
		self.next = next
	def clear(self):
		""" Очистка узла
		Задает поля класса значениями по умолчанию """
		self.data = 0
		self.next = None
	def __str__(self):
		""" Перегрузка метода
		Переопределяет метод str() для узла """
		return f"Node({self.data})"

class DoubleNode(Node):
	""" Двойной узел. Наследуется от Node
	Задается двойной узел. Методы
	__init__ Инициализатор """
	def __init__(self, data = 0, next = None, prev = None):
		""" Инициализатор двойного узла
		Поля: data
		      next
		      prev - ссылка на предыдущий узел """
		super().__init__(data, next)
		self.prev = prev
	def __str__(self):
		return f"Node({self.data})"

class SinglyLinkedList:
	""" Односсылочный список узлов(ОСУ, список)
	Задается ОСУ. Методы:
	__init__ Инициализатор
	prepend Добавить в начало
	append Добавить в конец
	insert_after вставить после элемента
	insert_before вставить до элемента
	remove_first убрать первый
	remove_last убрать последний
	remove_at убрать элемент по индексу
	remove_value убрать первое вхождение значения
	find найти первое вхождение значения
	find_all найти все вхождения значения
	contains проверка вхождения по значению
	get получить значение узла по индексу
	__len__ перегрузка метода len()
	__str__ перегрузка метода str()
	__iter__ делает список итерируемым
	clear очистка списка
------------------------
	__getitem__
	__setitem__
	__contains__
	__add__
	__mul__ """
	def __init__(self):
		""" Итератор списка
		Поля:
			head - ссылка на "головной" узел
			size - размер списка """
		self.head = None
		self.size = 0
	def prepend(self, data):
		"""Добавить в начало
		Добавляет новый узел в начало по значению"""
		new_node = Node(data, self.head)
		self.head = new_node
		self.size += 1
	def append(self, data):
		"""Добавить в конец
		Добавляет в конец по значению"""
		new_node = Node(data)
		if self.head is None:
			self.head = new_node
		else:
			current = self.head
			while current.next is not None:
				current = current.next
			current.next = new_node
		self.size += 1
	def insert_after(self, index, data):
		"""Вставить после элемента
		Вставляет узел со значением data после
		элемента с индексом index. Обрабатывает ошибку индекса """
		if index < 0 or index >= self.size:
			raise IndexError(index, "Index out of range(insert_after)")
		elif index == self.size - 1:
			self.append(data)
		else:
			current = self.head
			for _ in range(index):
				current = current.next
			new_node = Node(data, current.next)
			current.next = new_node
			self.size += 1

	def insert_before(self, index, data):
		"""Вставить до элемента
		Вставляет узел со значением data после
		элемента с индексом index. Обрабатывает ошибку индекса """
		if index < 0 or index >= self.size:
			raise IndexError(index, "Index out of range(insert_before)")
		elif index == 0:
			self.prepend(data)
		else:
			current = self.head
			for _ in range(index - 1):
				current = current.next
			new_node = Node(data, current.next)
			current.next = new_node
			self.size += 1
	def remove_first(self):
		"""Удалить первый
		Удаляет первый узел списка.Обрабатывает
		ошибку пустого объекта"""
		if self.head is None:
			raise EmptyStructureError("SinglyLinkedList", "rm first from empty")
		else:
			self.head = self.head.next
			self.size -= 1
	def remove_last(self):
		"""Удалить последний
		Удаляет последний узел списка. Обрабатывает
		ошибку пустого объекта"""
		if self.head is None:
			raise EmptyStructureError("SinglyLinkedList", "rm last from empty")
		else:
			current = self.head
			while current.next.next is not None:
				current = current.next
			current.next = None
			self.size -= 1
	def remove_at(self, index):
		"""Удалить по индексу
		Удаляет элемент по индексу. Обрабатывает
		ошибку индекса"""
		if index < 0 or index >= self.size:
			raise IndexError(index, "Index out of range(remove_at)")
		elif index == 0:
			self.remove_first()
		elif index == self.size - 1:
			self.remove_last()
		else:
			current = self.head
			for _ in range(index - 1):
				current = current.next
			current.next = current.next.next
			self.size -= 1
	def remove_value(self, value):
		"""Удалить по значению
		Удаляет первое вхождение значения.Обрабатывает
		ошибку пустого объекта"""
		if self.head == None:
			raise EmptyStructureError("SinglyLinkedList", "rm value from empty")
		else:
			current = self.head
			i = -1
			ch = False
			while current is not None:
				i += 1
				if current.data == value:
					ch = True
					break
				current = current.next
			if not ch:
				raise ValueNotFoundError(value, "rm value(value not found)")
			else:
				self.remove_at(i)
	def find(self, value):
		"""Найти
		Возвращает индекс первого вхождения по значению"""
		if self.head == None:
			return -1
		else:
			i = -1
			ch = False
			current = self.head
			while current is not None:
				i += 1
				if current.data == value:
					ch = True
					break
				current = current.next
			if ch:
				return i
			else:
				return -1
	def find_all(self, value):
		"""Найти все
		Возвращает индексы всех вхождений по значению"""
		if self.head == None:
			return []
		else:
			i = -1
			current = self.head
			indices = []
			while current is not None:
				i += 1
				if current.data == value:
					indices.append(i)
				current = current.next
			return indices
	def contains(self, value):
		"""Содержит
		Возвращает True или False, если
		список содержит или не содержит элемент со значением
		value соответственно"""
		if self.find(value) != -1:
			return True
		else:
			return False
	def get(self, index):
		"""Получить
		Возвращает информацию узла по индексу. Обрабатывает
		ошибку индекса"""
		if index < 0 or index >= self.size:
			raise IndexError(index, "Index out of range in get()")
		else:
			current = self.head
			for _ in range(index):
				current = current.next
			return current.data
	def __len__(self):
		"""Перегрузка len()"""
		return self.size
	def __str__(self):
		"""Перегрузка str()"""
		s = "None"
		current = self.head
		while current is not None:
			s += f" <- {current.data}"
			current = current.next
		return s
	def __iter__(self):
		"""Делает список итерируемым объектом"""
		current = self.head
		while current is not None:
			yield current
			current = current.next
	def clear(self):
		"""Очистить
		Задает поля head, size значениями
		по умолчанию None, 0 соответственно"""
		self.head = None
		self.size = 0
	def __getitem__(self, index):
		"""Перегрузка []
		Возвращает data узла по индексу"""
		return self.get(index)
	def __setitem__(self, index, item):
		"""Перегрузка
		Меняет значение узла по индексу"""
		current = self.head
		for _ in range(index):
			current = current.next
		current.data = item
	def __contains__(self, value):
		"""Перегрузка
		Возвращает True или False если список содержит или не содержит
		соотвественно значение"""
		return self.contains(value)
	def __add__(list1, list2):
		"""Перегрузка конкатенации
		Второй список приписывается в конец первого"""
		datas = []
		for i in range(len(list2)):
			datas.append(list2[i])
		for x in datas:
			list1.append(x)
		return list1
	def __mul__(lst, n):
		"""Перегрузка сложения списков
		Складывает списки"""
		list_res = SinglyLinkedList()
		for i in range(n):
			list_res = list_res + lst
		return list_res

class DoublyLinkedList:
	"""Двуссылочный список узлов(ОСУ, список)
	Задается ОСУ. Методы:
	init__ Инициализатор
	prepend Добавить в начало
	append Добавить в конец
	insert_after вставить после элемента
	insert_before вставить до элемента
	remove_first убрать первый
	remove_last убрать последний
	remove_at убрать элемент по индексу
	remove_value убрать первое вхождение значения
	find найти первое вхождение значения
	find_all найти все вхождения значения
	contains проверка вхождения по значению
	get получить значение узла по индексу
	__len__ перегрузка метода len()
	__str__ перегрузка метода str()
	__iter__ делает список итерируемым
	clear очистка списка
	-----------------------------------
	__getitem__
	__setitem__
	__contains__
	__add__
	__mul__ """

	def __init__(self):
		"""Итератор списка
		Поля:
			head - ссылка на "головной" узел
			size - размер списка
			tail - ссылка на "хвостовой" узел """
		self.head = None
		self.tail = None
		self.size = 0 #не было написано, что нужно добавлять, но в singlylinked было
	def prepend(self, data):
		"""Добавить в начало
		Добавляет новый узел в начало по значению"""
		new_node = DoubleNode(data, self.head, None)
		if self.head is None:
			self.head = new_node
			self.tail = new_node
		else:
			self.head.prev = new_node
			self.head = new_node
		self.size += 1
	def append(self, data):
		"""Добавить в конец
		Добавляет в конец по значению"""
		new_node = DoubleNode(data, None, self.tail)
		if self.head is None:
			self.head = new_node
			self.tail = new_node
		else:
			self.tail.next = new_node
			self.tail = new_node
		self.size += 1
	def insert_after(self, index, data):
		"""Вставить после элемента
		Вставляет узел со значением data после
		элемента с индексом index. Обрабатывает ошибку индекса """
		if index < 0 or index >= self.size:
			raise IndexError(index, "Index out of range(Double insert_after)")
		elif index == self.size - 1:
			self.append(data)
		else:
			current = self.head
			for _ in range(index):
				current = current.next
			new_node = DoubleNode(data, current.next, current)
			current.next.prev = new_node
			current.next = new_node
			self.size += 1
	def insert_before(self, index, data):
		"""Вставить до элемента
		Вставляет узел со значением data после
		элемента с индексом index. Обрабатывает ошибку индекса """
		if index < 0 or index >= self.size:
			raise IndexError(index, "Index out of range(Double insert_before)")
		elif index == 0:
			self.prepend(data)
		else:
			current = self.head
			for _ in range(index - 1):
				current = current.next
			new_node = DoubleNode(data, current.next, current)
			current.next.prev = new_node
			current.next = new_node
			self.size += 1
	def remove_first(self):
		"""Удалить первый
		Удаляет первый узел списка.Обрабатывает
		ошибку пустого объекта"""
		if self.head is None:
			raise EmptyStructureError("DoublyLinkedList", "rm first from empty")
		else:
			self.head = self.head.next
			self.head.prev = None
			self.size -= 1
	def remove_last(self):
		"""Удалить последний
		Удаляет последний узел списка. Обрабатывает
		ошибку пустого объекта"""
		if self.head is None:
			raise EmptyStructureError("DoublyLinkedList", "rm last from empty")
		else:
			self.tail = self.tail.prev
			self.tail.next = None
			self.size -= 1
	def remove_at(self, index):
		"""Удалить по индексу
		Удаляет элемент по индексу. Обрабатывает
		ошибку индекса"""
		if index < 0 or index >= self.size:
			raise IndexError(index, "Index out of range(Double remove_at)")
		elif index == 0:
			self.remove_first()
		elif index == self.size - 1:
			self.remove_last()
		else:
			current = self.head
			for _ in range(index - 1):
				current = current.next
			current.next = current.next.next
			current.next.prev = current
			self.size -= 1
	def remove_value(self, value):
		"""Удалить по значению
		Удаляет первое вхождение значения.Обрабатывает
		ошибку пустого объекта"""
		if self.head == None:
			raise EmptyStructureError("DoublyLinkedList", "rm value from empty")
		else:
			current = self.head
			i = -1
			ch = False
			while current is not None:
				i += 1
				if current.data == value:
					ch = True
					break
				current = current.next
			if not ch:
				raise ValueNotFoundError(value, "Double rm value(value not found)")
			else:
				self.remove_at(i)
	def find(self, value):
		"""Найти
		Возвращает индекс первого вхождения по значению"""
		if self.head == None:
			return -1
		else:
			i = -1
			ch = False
			current = self.head
			while current is not None:
				i += 1
				if current.data == value:
					ch = True
					break
				current = current.next
			if ch:
				return i
			else:
				return -1
	def find_all(self, value):
		"""Найти все
		Возвращает индексы всех вхождений по значению"""
		if self.head == None:
			return []
		else:
			i = -1
			current = self.head
			indices = []
			while current is not None:
				i += 1
				if current.data == value:
					indices.append(i)
				current = current.next
			return indices
	def contains(self, value):
		"""Содержит
		Возвращает True или False, если
		список содержит или не содержит элемент со значением
		value соответственно"""
		if self.find(value) != -1:
			return True
		else:
			return False
	def get(self, index):
		"""Получить
		Возвращает информацию узла по индексу. Обрабатывает
		ошибку индекса"""
		if index < 0 or index >= self.size:
			raise IndexError(index, "Index out of range in get()")
		else:
			current = self.head
			for _ in range(index):
				current = current.next
			return current.data
	def __len__(self):
		"""Перегрузка len()"""
		return self.size
	def __str__(self):
		"""Перегрузка str()"""
		s = "None"
		current = self.head
		if self.head is None:
			return s
		else:
			s += " <-"
			while current is not None:
				s += f" {current.data} <->"
				current = current.next
			return s[:-4] + f" -> None"
	def __iter__(self):
		"""Делает список итерируемым объектом"""
		current = self.head
		while current is not None:
			yield current
			current = current.next
	def clear(self):
		"""Очистить
		Задает поля head, size значениями
		по умолчанию None, 0 соответственно"""
		self.head = None
		self.tail = None
		self.size = 0
	def __getitem__(self, index):
		"""Перегрузка []
		Возвращает data узла по индексу"""
		return self.get(index)
	def __setitem__(self, index, item):
		"""Перегрузка
		Меняет значение узла по индексу"""
		current = self.head
		for _ in range(index):
			current = current.next
		current.data = item
	def __contains__(self, value):
		"""Перегрузка
		Возвращает True или False если список содержит или не содержит
		соотвественно значение"""
		return self.contains(value)
	def __add__(list1, list2):
		"""Перегрузка конкатенации
		Второй список приписывается в конец первого"""
		datas = []
		for i in range(len(list2)):
			datas.append(list2[i])
		for x in datas:
			list1.append(x)
		return list1
	def __mul__(lst, n):
		"""Перегрузка сложения списков
		Складывает списки"""
		list_res = DoublyLinkedList()
		for i in range(n):
			list_res = list_res + lst
		return list_res

class Stack:
	"""Стек. Композиция SinglyLinkedList
	Методы:
	__init__ Инициализатор класса
	push Добавить в стек
	pop Удалить элемент и показать его
	peek Показать элемент
	is_empty Проверить на пустоту
	size Получить размер
	from_queue Преобразовать очередь в стек"""
	def __init__(self):
		"""Инициализатор стека
		Поля: items"""
		self.items = SinglyLinkedList()
	def push(self, data):
		"""Добавить элемент
		Добавляет элемент наверх стека"""
		self.items.prepend(data)
	def pop(self):
		"""Удалить элемент и показать его
		Удаляет последний элемент и показывает его данные"""
		if len(self.items) == 0:
			raise EmptyStructureError("Stack", "pop from empty")
		else:
			data = self.items.get(0)
			self.items.remove_first()
			return data
	def peek(self):
		"""Показать элемент
		Показывает последний элемент"""
		if len(self.items) == 0:
			raise EmptyStructureError("Stack", "peek from empty")
		else:
			return self.items.get(0)
	def is_empty(self):
		"""Проверка на пустоту стека
		Проверяет на пустоту: пустой - True, нет - False"""
		return True if len(self.items) == 0 else False
	def size(self):
		"""Получить размер
		Возвращает размер стека"""
		return len(self.items)
	@staticmethod
	def from_queue(queue):
		"""Преобразовать очередь в стек
		Преобразовывает очередь в стек. Статический метод"""
		stack = Stack()
		list_o_data = []
		for i in range(queue.size()):
			list_o_data.append(queue.front())
		for x in list_o_data:
			stack.push(x)
		return stack

class Queue:
	"""Очередь. Композиция класса SinglyLinkedList
	Методы:
	__init__ Инициализатор
	enqueue Добавить в конец очереди
	dequeue Удалить из начала
	front Показать первый
	is_empty Проверка на пустоту
	size Получить размер
	from_stack Преобразовать стек в очередь"""
	def __init__(self):
		"""Инициализатор очереди
		Поля: items"""
		self.items = SinglyLinkedList()
	def enqueue(self, data):
		"""Добавить в конец
		Добавляет элемент в конец очереди"""
		self.items.append(data)
	def dequeue(self):
		"""Убрать элемент
		Убирает первый элемент очереди. Обрабатывает ошибку пустой структуры"""
		if len(self.items) == 0:
			raise EmptyStructureError("Queue", "dequeue from empty")
		else:
			data = self.items.get(0)
			self.items.remove_first()
			return data
	def front(self):
		"""Показать первый
		Показывает первый элемент очереди"""
		if len(self.items) == 0:
			raise EmptyStructureError("Queue", "front from empty")
		else:
			return self.items.get(0)
	def is_empty(self):
		"""Проверка пустоты
		Если очередь пустая - True, иначе - False"""
		return True if len(self.items) == 0 else False
	def size(self):
		"""Получить размер
		Возвращает размер очереди"""
		return len(self.items)
	@staticmethod
	def from_stack(stack):
		"""Преобразовать стек в очередь
		Преобразовывает стек в очередь. Статический метод"""
		queue = Queue()
		list_o_data = []
		for i in range(stack.size()):
			list_o_data.append(stack.pop())
		for x in reversed(list_o_data):
			queue.enqueue(x)
		return queue
