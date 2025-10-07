import pandas as pd

try:
	# Lee el archivo CSV
	df = pd.read_csv('calibration.csv')

	# Imprime la lista en el formato deseado
	print("{")
	for index, row in df.iterrows():
		color = row['color']
		values = row[1:].tolist()

		values_str = str(values).replace('[', '{').replace(']', '}')
		print(f"    {{ \"{color}\", {values_str} }},")
	print("}")

except FileNotFoundError:
	print("El archivo 'calibration.csv' no se encontró.")
	print("Por favor, asegúrate de que el archivo se haya cargado correctamente.")