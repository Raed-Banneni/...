from django.shortcuts import render, redirect 
from django.contrib.auth import authenticate, login, logout, update_session_auth_hash 
from django.contrib.auth.forms import UserCreationForm, UserChangeForm, PasswordChangeForm
from django.contrib import messages 
from .forms import SignUpForm, EditProfileForm , familleForm 
from .models import famille
from django.views.generic.edit import CreateView, DeleteView
from django.urls import reverse_lazy
import gspread
from oauth2client.service_account import ServiceAccountCredentials
from django.utils import timezone
from .models import DerniereLigne
from django.urls import reverse
from .models import HeartRate
from django.core.mail import send_mail
from django.conf import settings
from django.contrib.auth.models import User
from django.http import HttpResponse
from rest_framework import status,filters
from rest_framework.response import Response
from rest_framework.decorators import api_view
from .serializers import HeartRateSerializer
import plotly.graph_objs as go
from django.http import JsonResponse
from datetime import timedelta





@api_view(['GET', 'POST', 'DELETE'])
def API_List(request):
    # GET
    if request.method == 'GET':
        HeartRates = HeartRate.objects.all()
        serializer = HeartRateSerializer(HeartRates, many=True)
        return Response(serializer.data)
    # POST
    elif request.method == 'POST':
        serializer = HeartRateSerializer(data=request.data)
        if serializer.is_valid():
            serializer.save()
            return Response(serializer.data, status=status.HTTP_201_CREATED)
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
    # DELETE
    elif request.method == 'DELETE':
        HeartRate.objects.all().delete()
        return Response(status=status.HTTP_204_NO_CONTENT)

# Create your views here.

def home(request): 
    liste_familles = famille.objects.all()
    context = {"liste_familles": liste_familles}
    return render(request, 'authenticate/home.html', context)

def login_user (request):
	if request.method == 'POST': #if someone fills out form , Post it 
		username = request.POST['username']
		password = request.POST['password']
		user = authenticate(request, username=username, password=password)
		if user is not None:# if user exist
			login(request, user)
			messages.success(request,('Vous étes connectés'))
			return redirect('home') #routes to 'home' on successful login  
		else:
			messages.success(request,('Erreur de connexion'))
			return redirect('login') #re routes to login page upon unsucessful login
	else:
		return render(request, 'authenticate/login.html', {})

def logout_user(request):
	logout(request)
	messages.success(request,('Vous êtes maintenant déconnecté'))
	return redirect('login')

def register_user(request):
	if request.method =='POST':
		form = SignUpForm(request.POST)
		if form.is_valid():
			form.save()
			username = form.cleaned_data['username']
			password = form.cleaned_data['password1']
			user = authenticate(username=username, password=password)
			login(request,user)
			messages.success(request, ('Vous êtes maintenant inscrit'))
			return redirect('home')
	else: 
		form = SignUpForm() 

	context = {'form': form}
	return render(request, 'authenticate/register.html', context)

def edit_profile(request):
	if request.method =='POST':
		form = EditProfileForm(request.POST, instance= request.user)
		if form.is_valid():
			form.save()
			messages.success(request, ('Vous avez modifié votre profil'))
			return redirect('home')
	else: 		#passes in user information 
		form = EditProfileForm(instance= request.user) 

	context = {'form': form}
	return render(request, 'authenticate/edit_profile.html', context)
	#return render(request, 'authenticate/edit_profile.html',{})



def change_password(request):
	if request.method =='POST':
		form = PasswordChangeForm(data=request.POST, user= request.user)
		if form.is_valid():
			form.save()
			update_session_auth_hash(request, form.user)
			messages.success(request, ('Vous avez modifié votre mot de passe'))
			return redirect('home')
	else: 		#passes in user information 
		form = PasswordChangeForm(user= request.user) 

	context = {'form': form}
	return render(request, 'authenticate/change_password.html', context)
def analyse(request, id_famille):
    famille_obj = famille.objects.get(id=id_famille)
    HeartRates = HeartRate.objects.filter(famille=famille_obj)
    bpms = [hr.bpm for hr in HeartRates]
    spo2s = [hr.spo2 for hr in HeartRates]
    timestamps = [hr.timestamp for hr in HeartRates]
    for i in range(len(timestamps)):
        timestamps[i] = timestamps[i] + timedelta(hours=1)
    data = [
        go.Scatter(x=timestamps, y=bpms, name='Bpm'),
        go.Scatter(x=timestamps, y=spo2s, name='SpO2', text=[f'{spo2}% ' for spo2 in spo2s])
    ]
    layout = go.Layout(
        title={
            'text': 'Rythme cardiaque et SpO2 (par jour)',
            'x': 0.6, # décaler à droite
            'xanchor': 'right' # ancrer à droite
        },
        font=dict(
            family='Arial, sans-serif', # police de caractères
            size=18, # taille de la police
            color='black', # couleur de la police
        )
    )
    xaxis=dict(title='Mesuré à'),
    yaxis=dict(title='Mesure')

    fig = go.Figure(data=data, layout=layout)
    comment = ""
    scope = ["***********************************","**********************","***********************"]
    creds = ServiceAccountCredentials.from_json_keyfile_name('C:\\Users\\joseph\\Desktop\\User-Authentication-Login-Register-Logout-Python-Django--master\\creds\\creds', scope)
    client = gspread.authorize(creds)
    sheet = client.open('pfa1').sheet1
    data = sheet.get_all_records()
    # Récupérer les données de la colonne 1
    column_1 = sheet.col_values(1)
    for i in range(len(column_1)):
        if column_1[i] == 'chute':
            column_1[i] = 'aucune information'
    # Récupérer la dernière ligne de la colonne 1
    last_row = column_1[-1]
    # Afficher la dernière ligne
    #print(last_row)
    derniere_ligne = DerniereLigne.objects.filter(famille=famille_obj, nom=last_row).first()
    if derniere_ligne:
        # La dernière ligne a été enregistrée dans la base de données, donc nous envoyons un e-mail de notification à l'utilisateur
        subject = 'Nouvelle Alerte'
        message = f'votre patient ({famille_obj.prenom}) a tombé le ({last_row}).'
        #email_from = settings.EMAIL_HOST_USER
        user = request.user
        email_from = user.email
        recipient_list = [famille_obj.email_pour_notification]
        send_mail(subject, message, email_from, recipient_list)
    else:
        # La dernière ligne n'a pas encore été enregistrée dans la base de données, donc nous l'ajoutons maintenant
        derniere_ligne = DerniereLigne.objects.create(famille=famille_obj, nom=last_row, date=timezone.now())
    context = {"famille": famille_obj, "data":last_row,"chart": fig.to_html()}
    return render(request, 'authenticate/analyse.html', context)
    

    


def historique(request, id_famille):
    liste_DerniereLigne = DerniereLigne.objects.all()
    context = {"liste_DerniereLigne": liste_DerniereLigne}
    return render(request, 'authenticate/historique.html', context)





class Addfamille(CreateView):
    model = famille
    form_class = familleForm
    template_name = "ajouter_famille.html"
    success_url = "home"
    
    def form_valid(self, form):
        form.instance.user = self.request.user
        if form.is_valid():
            form.save()
            messages.success(self.request, ('/////////////Vous avez ajouté votre patient'))
        return super().form_valid(form)





   
class Deletefamille(DeleteView):
    model = famille
    template_name = "supprimer_famille.html"
    
    success_url = reverse_lazy("home")

    def get(self, request, *args, **kwargs):
        return self.post(request, *args, **kwargs)

    def delete(self, request, *args, **kwargs):
        self.object = self.get_object()
        self.object.delete()
        return super().delete(request, *args, **kwargs)
    
	



#class HeartRateView(View):
 #   def post(self, request):
  #      bpm = request.POST.get('bpm')
   #     heart_rate = HeartRate(bpm=bpm)
    #    heart_rate.save()
     #   return render(request, 'analyse.html', {'bpm': bpm})




