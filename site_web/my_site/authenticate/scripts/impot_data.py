from authenticate.models import famille

def run():
    for i in range(1,2):
        new_famille = famille()
        new_famille.nom = "nom N° #%d" %i 
        new_famille.prenom = "prenom N° #%d" %i 
        new_famille.image = "http://default"
        new_famille.save()
    print("operation réussie")

