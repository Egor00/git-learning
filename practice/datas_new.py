import hunspell
import cv2
import numpy as np
import os
from huggingface_hub import hf_hub_download
from PIL import Image
from transformers import TrOCRProcessor, VisionEncoderDecoderModel
from matplotlib import pyplot as plt

def Otsu(row_sums):
    """
    Находит оптимальный порог для разделения строк с текстом и пустых строк
    методом Otsu (адаптирован для одномерного массива).
    """
    data = row_sums.astype(np.uint32)
    hist, bin_edges = np.histogram(data, bins='auto')
    hist = hist.astype(np.float32)
    hist /= hist.sum()
    cum_sum = np.cumsum(hist)
    cum_mean = np.cumsum(hist * bin_edges[:-1])
    total_mean = cum_mean[-1]
    max_variance = 0
    best_threshold = 0
    for t in range(1, len(bin_edges)-1):
        w0 = cum_sum[t]
        w1 = 1 - w0
        if w0 == 0 or w1 == 0:
            continue
        mean0 = cum_mean[t] / w0
        mean1 = (total_mean - cum_mean[t]) / w1
        variance = w0 * w1 * ((mean0 - mean1) ** 2)
        if variance > max_variance:
            max_variance = variance
            best_threshold = bin_edges[t]
    return best_threshold

def text_noise(image_inverted, fins):
    fins_n = []
    for y1, y2 in fins:
        cropped = image_inverted[max(0, y1-5):min(2590, y2+5), :]
        s = 0
        for i in cropped.flatten():
            if i != 0:
                s+=1
        h, w = cropped.shape
        if 0.02 <= s/(h*w) <= 0.45:
            fins_n.append((y1, y2))
    return fins_n

def main():
# Загружаем изображение "/home/pencil/Загрузки/practice/-403286322_02 (копия).png"
    ''' Загрузка и обработка фото'''
    path_img = input("Введите путь до изображения:\n")
    image = cv2.imread(path_img, cv2.IMREAD_GRAYSCALE)
    inverted_image = 255-image
    '''Сумма пикселей текста по строкам пикселей'''
    weight_lines = np.sum(inverted_image, axis=1)
    '''1)Число пикселей в строке, больше которого - не фоновый шум
       2)+ поиск нужных строк пикселей
       3)+ разбиение их на строки с текстом
       4)+ проверка на шум'''
    threshold = Otsu(weight_lines) #1)
    lines = np.where(weight_lines > threshold)[0] #2)
    #3)
    start = lines[0]
    prev = start
    fins = []
    for row in lines[1:]:
        if row - prev > 10:
            fins.append((start, prev+1))
            start = row
        prev = row
    fins.append((start, prev+1))
    lines_coords = text_noise(inverted_image, fins) #4)
    ''' Загрузка модели и процессора '''
    processor = TrOCRProcessor.from_pretrained("raxtemur/trocr-base-ru")
    model = VisionEncoderDecoderModel.from_pretrained("raxtemur/trocr-base-ru")
    h = hunspell.HunSpell('/home/pencil/dicts/ru_RU.dic', '/home/pencil/dicts/ru_RU.aff')
    ''' Вычленение имени файла и пр.'''
    s = path_img+'\n'
    name = path_img.split('/')[-1].split('.')[0]+'.txt'
    ''' Разбиение текста по строкам и чтение с фото '''
    print(f"Количество строк: {len(lines_coords)}")
    l = 1
    for y1, y2 in lines_coords:
        cropped_line = image[max(0, y1-10):min(2590, y2+10), :]
        pil_image = Image.fromarray(cv2.cvtColor(cropped_line, cv2.COLOR_GRAY2RGB))
       #plt.figure(figsize=(15,20))
       #plt.imshow(pil_image)
       #plt.show()
        pixel_values = processor(images=pil_image, return_tensors="pt").pixel_values.to(model.device)
        generated_ids = model.generate(pixel_values)
        generated_text = processor.batch_decode(generated_ids, skip_special_tokens=True)[0]
        for w in generated_text.split(' '):
            suggest = h.suggest(w)
            if suggest and suggest[0] != w:
                #s += suggest[0] + ' '
                s += w + ' '
            else:
                s += w + ' '
        print(f"Строка номер {l} прочитана. Осталось {len(lines_coords)-l}.")
        l += 1
        s += '\n'
    print(s)
    ''' Загрузка в файл '''
    with open(f'/home/pencil/результат/{name}', 'w', encoding='utf-8') as file:
        pass
    with open(f'/home/pencil/результат/{name}', 'r+', encoding='utf-8') as file:
        file.write(s)
        print(file.read())

if __name__== "__main__":
    main()
