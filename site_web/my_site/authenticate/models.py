from django.db import models

class famille(models.Model):

    id = models.BigAutoField(primary_key=True)
    nom = models.CharField(max_length=255)
    prenom=models.CharField(max_length=50)
    image=models.ImageField(null=True, blank=True)
    created_at=models.DateTimeField(auto_now_add=True)
    email_pour_notification = models.EmailField(max_length=254)

    def __str__(self):
        return self.prenom


class DerniereLigne(models.Model):
    nom = models.CharField(max_length=200)
    date = models.DateTimeField(auto_now_add=True)
    famille = models.ForeignKey(famille, on_delete=models.CASCADE)

    def __str__(self):
        return self.nom



class HeartRate(models.Model):
    timestamp = models.DateTimeField(auto_now_add=True)
    bpm = models.IntegerField()
    spo2 = models.IntegerField()
    famille = models.ForeignKey(famille, related_name='famille', on_delete=models.CASCADE)
    
    def __str__(self):
        return f"Heart rate "
