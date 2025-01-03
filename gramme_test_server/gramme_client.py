import gramme

client = gramme.client(host="192.168.11.3", port=8080)

some_data = {'key': 'value'}
client.send(some_data)

more_data = ['i am a list', 1, {'hello': 'there!'}]
client.send(more_data)